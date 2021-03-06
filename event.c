/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <stdlib.h>
#include "nilwm.h"

#define MOD_MASK_(state)    ((state) & ~(nil_.mask_numlock | XCB_MOD_MASK_LOCK))

static struct mouse_event_t mouse_evt_;

static
void handle_key_press(xcb_key_press_event_t *e) {
    xcb_keysym_t sym;
    NIL_LOG("event: key press %d %d", e->state, e->detail);

    sym = xcb_key_symbols_get_keysym(nil_.key_syms, e->detail, 0);
    /* find key with *LOCK state removed */
    if (check_key(MOD_MASK_(e->state), sym)) {
        xcb_flush(nil_.con);
        return;
    }
}

static
void handle_key_release(xcb_key_release_event_t *e) {
    NIL_LOG("event: key release %d %d", e->state, e->detail);
}

/** Handle the ButtonPress event
 */
static
void handle_button_press(xcb_button_press_event_t *e) {
    NIL_LOG("event: mouse %d pressed: event %d, child %d (%d,%d)",
        e->detail, e->event, e->child, e->event_x, e->event_y);

    if (e->event == bar_.win) {
        if (click_bar(e->event_x)) {
            xcb_flush(nil_.con);
        }
        return;
    }
    /* click on client with modkey */
    if (MOD_MASK_(e->state) == cfg_.mod_key) {
        mouse_evt_.client = find_client(e->child, &mouse_evt_.ws);
        if (!mouse_evt_.client) {
            NIL_ERR("no client %d", e->child);
            goto end;
        }
        NIL_LOG("button on win=%d", mouse_evt_.client->win);
        switch (e->detail) {
        case XCB_BUTTON_INDEX_3:
            mouse_evt_.mode = CURSOR_RESIZE;
            /* warp pointer to lower right */
            mouse_evt_.x1 = mouse_evt_.client->x + mouse_evt_.client->w;
            mouse_evt_.y1 = mouse_evt_.client->y + mouse_evt_.client->h;
            xcb_warp_pointer(nil_.con, XCB_NONE, mouse_evt_.client->win,
                0, 0, 0, 0, mouse_evt_.x1, mouse_evt_.y1);
            break;
        case XCB_BUTTON_INDEX_1:
        default:
            mouse_evt_.mode = CURSOR_MOVE;
            mouse_evt_.x1 = e->event_x;
            mouse_evt_.y1 = e->event_y;
            break;
        }
        /* take control of the pointer in the root window */
        xcb_grab_pointer(nil_.con, 0, nil_.scr->root,
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION,
            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE,
            nil_.cursor[mouse_evt_.mode], XCB_CURRENT_TIME);
        xcb_flush(nil_.con);
        return;
    }
end:
    /* if unhandled, forward the click to the application */
    xcb_allow_events(nil_.con, XCB_ALLOW_REPLAY_POINTER, e->time);
}

static
void handle_button_release(xcb_button_release_event_t *e) {
    const struct layout_t *h;

    NIL_LOG("event: mouse release %d", e->detail);
    mouse_evt_.x2 = e->event_x;
    mouse_evt_.y2 = e->event_y;

    h = get_layout(mouse_evt_.ws);
    switch (mouse_evt_.mode) {
    case CURSOR_MOVE:
        if (h->move) {
            (*h->move)(mouse_evt_.ws, &mouse_evt_);
        }
        break;
    case CURSOR_RESIZE:
        if (h->resize) {
            (*h->resize)(mouse_evt_.ws, &mouse_evt_);
        }
        break;
    }
    xcb_ungrab_pointer(nil_.con, XCB_CURRENT_TIME);
    xcb_flush(nil_.con);
}

/** Mouse moved
 */
static
void handle_motion_notify(xcb_motion_notify_event_t *e) {
    NIL_LOG("event: mouse motion %d,%d", e->event_x, e->event_y);

    mouse_evt_.x2 = e->event_x;
    mouse_evt_.y2 = e->event_y;
}

/** Mouse entered
 */
static
void handle_enter_notify(xcb_enter_notify_event_t *e) {
    NIL_LOG("event: enter notify win=%d, child=%d mode=%d",
        e->event, e->child, e->mode);
#if 0
    if (e->mode == XCB_NOTIFY_MODE_NORMAL || e->mode == XCB_NOTIFY_MODE_UNGRAB) {
        struct client_t *c;
        struct workspace_t *ws;

        c = find_client(e->event, &ws);
        if (!c) {
            NIL_ERR("no client %d", e->event);
            return;
        }
        if (ws->focus) {
            blur_client(ws->focus);
        }
        focus_client(c);
        ws->focus = c;
        xcb_flush(nil_.con);
    }
#endif
}

/** Focused
 */
static
void handle_focus_in(xcb_focus_in_event_t *e) {
    struct client_t *c;
    struct workspace_t *ws;

    NIL_LOG("event: focus in win=%d", e->event);
    if (e->mode == XCB_NOTIFY_MODE_GRAB || e->mode == XCB_NOTIFY_MODE_UNGRAB
        || e->detail == XCB_NOTIFY_DETAIL_POINTER) {
        /* ignore event for grap/ungrap or detail is pointer */
        return;
    }
    c = find_client(e->event, &ws);
    if (!c) {
        NIL_ERR("no client %d", e->event);
        return;
    }
    if (c == ws->focus) {
        return;
    }
    if (ws->focus) {
        blur_client(ws->focus);
    }
    focus_client(c);
    ws->focus = c;
    xcb_flush(nil_.con);
}

/** Blured
 */
static
void handle_focus_out(xcb_focus_out_event_t *e) {
    NIL_LOG("event: focus out win=%d", e->event);
}

/** This handler just for status bar initialization.
 */
static
void handle_expose(xcb_expose_event_t *e) {
    NIL_LOG("event: expose win=%d %d,%d %ux%u", e->window, e->x, e->y, e->width,
        e->height);

    if (e->window == bar_.win) {            /* init bar */
        NIL_LOG("bar draw seq=%u", e->sequence);
        if (e->count == 0) {
            config_bar();
            update_bar_ws(nil_.ws_idx);
            update_bar_sym();
            update_bar_status();
            xcb_flush(nil_.con);
        }
        return;
    }
}

/** Handler for creating a new window
 * xterm size request is 1x1?
 */
static
void handle_create_notify(xcb_create_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("event: create notify win=%d par=%d @ %d,%d %ux%u", e->window,
        e->parent, e->x, e->y, e->width, e->height);
    if (e->override_redirect) {
        return;
    }
    c = malloc(sizeof(struct client_t));
    if (!c) {
        NIL_ERR("out of mem %d", e->window);
        return;
    }
    c->win = e->window;
    c->x = e->x;
    c->y = e->y;
    c->w = e->width;
    c->h = e->height;
    c->border_width = e->border_width;
    /* add into current workspace and re-arrange */
    attach_client(c, &nil_.ws[nil_.ws_idx]);
}

/** Handler for destroying a window
 */
static
void handle_destroy_notify(xcb_destroy_notify_event_t *e) {
    struct client_t *c;
    struct workspace_t *ws;

    NIL_LOG("event: destroy notify win=%d", e->window);
    c = find_client(e->window, &ws);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    detach_client(c);
    if (!NIL_HAS_FLAG(c->flags, CLIENT_FLOAT)) {
        /* rearrange if is current workspace */
        if (ws == &nil_.ws[nil_.ws_idx]) {
            arrange_ws(ws);
            xcb_flush(nil_.con);
        }
    }
    if (ws->focus == c) {
        ws->focus = 0;
    }
    free(c);
}

static
void handle_unmap_notify(xcb_unmap_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("event: unmap notify %d", e->window);
    c = find_client(e->window, 0);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    NIL_CLEAR_FLAG(c->flags, CLIENT_MAPPED);
}

static
void handle_map_notify(xcb_map_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("event: map notify %d", e->window);
    if (e->window == bar_.win) {
        return;
    }
    c = find_client(e->window, 0);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    NIL_SET_FLAG(c->flags, CLIENT_MAPPED);
}

/** Handler for changing position, size
 */
static
void handle_configure_notify(xcb_configure_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("event: configure notify evt=%d win=%d abv=%d @ %d,%d %ux%u",
        e->event, e->window, e->above_sibling, e->x, e->y, e->width, e->height);

    /* only recreate root background picture */
    if (e->window == nil_.scr->root) {
        return;
    }
#if 0
    c = find_client(e->window);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    /* update client's geometry */
    c->x = e->x;
    c->y = e->y;
    /* invalidate pixmap if resized */
    if (c->w != e->width || c->h != e->height
        || c->border_width != e->border_width) {
    }
    c->w = e->width;
    c->h = e->height;
    c->border_width = e->border_width;
    /* restack */
#else
    (void)c;
#endif
}

static
void handle_map_request(xcb_map_request_event_t *e) {
    xcb_get_window_attributes_reply_t *reply;
    struct client_t *c;
    struct workspace_t *ws;

    NIL_LOG("event: map request win=%d", e->window);
    reply = xcb_get_window_attributes_reply(nil_.con,
        xcb_get_window_attributes_unchecked(nil_.con, e->window), 0);
    if (!reply) {
        NIL_ERR("no reply %d", e->window);
        return;
    }
    if (reply->override_redirect) {
        NIL_ERR("override_redirect %d", e->window);
        free(reply);
        return;
    }
    free(reply);
    c = find_client(e->window, &ws);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    init_client(c);
    NIL_SET_FLAG(c->flags, CLIENT_DISPLAY);
    if (!NIL_HAS_FLAG(c->flags, CLIENT_FLOAT)) {
        /* only rearrange if it's not float */
        arrange_ws(ws);
    }
    if (check_client_size(c)) {             /* fix window size if needed */
        update_client_geom(c);
    }
    config_client(c);
    if (ws == &nil_.ws[nil_.ws_idx]) {
        xcb_map_window(nil_.con, c->win);
        xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT, c->win,
            XCB_CURRENT_TIME);
    }
    xcb_flush(nil_.con);
}

static
void handle_property_notify(xcb_property_notify_event_t *e) {
    NIL_LOG("event: property notify win=%d atom=%d", e->window, e->atom);

    if (e->atom == XCB_ATOM_WM_NAME && e->window == nil_.scr->root) {
        update_bar_status();
        xcb_flush(nil_.con);
        return;
    }
}

typedef void (*event_handler_t)(xcb_generic_event_t *);
static const event_handler_t HANDLERS_[] = {
    [XCB_KEY_PRESS]         = (event_handler_t)&handle_key_press,
    [XCB_KEY_RELEASE]       = (event_handler_t)&handle_key_release,
    [XCB_BUTTON_PRESS]      = (event_handler_t)&handle_button_press,
    [XCB_BUTTON_RELEASE]    = (event_handler_t)&handle_button_release,
    [XCB_MOTION_NOTIFY]     = (event_handler_t)&handle_motion_notify,
    [XCB_ENTER_NOTIFY]      = (event_handler_t)&handle_enter_notify,
    [XCB_FOCUS_IN]          = (event_handler_t)&handle_focus_in,
    [XCB_FOCUS_OUT]         = (event_handler_t)&handle_focus_out,
    [XCB_EXPOSE]            = (event_handler_t)&handle_expose,
    [XCB_CREATE_NOTIFY]     = (event_handler_t)&handle_create_notify,
    [XCB_DESTROY_NOTIFY]    = (event_handler_t)&handle_destroy_notify,
    [XCB_UNMAP_NOTIFY]      = (event_handler_t)&handle_unmap_notify,
    [XCB_MAP_NOTIFY]        = (event_handler_t)&handle_map_notify,
    [XCB_MAP_REQUEST]       = (event_handler_t)&handle_map_request,
    [XCB_CONFIGURE_NOTIFY]  = (event_handler_t)&handle_configure_notify,
    [XCB_PROPERTY_NOTIFY]   = (event_handler_t)&handle_property_notify,
};

/** Events loop
 */
void recv_events() {
    xcb_generic_event_t *e;
    unsigned int type;
    while ((e = xcb_wait_for_event(nil_.con))) {
        type = e->response_type & ~0x80;
        if (type < NIL_LEN(HANDLERS_) && HANDLERS_[type] != 0) {
            (*HANDLERS_[type])(e);
        } else {
            NIL_LOG("event: unknown type %u", type);
        }
        /* Free the Generic Event */
        free(e);
    }
}
/* vim: set ts=4 sw=4 expandtab: */
