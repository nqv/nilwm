/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>
#include "nilwm.h"

static
void handle_key_press(xcb_key_press_event_t *e) {
    xcb_keysym_t sym;
    NIL_LOG("key press %d %d", e->state, e->detail);

    sym = get_keysym(e->detail, e->state);
    if (check_key(e->state, sym)) {
        return;
    }
}

static
void handle_key_release(xcb_key_release_event_t *e) {
    NIL_LOG("key release %d %d", e->state, e->detail);
}

/** Handle the ButtonPress event
 */
static
void handle_button_press(xcb_button_press_event_t *e) {
    NIL_LOG("mouse %d pressed: event %d, child %d (%d,%d)",
        e->detail, e->event, e->child, e->event_x, e->event_y);

    /* if unhandled, forward the click to the application */
    xcb_allow_events(nil_.con, XCB_ALLOW_REPLAY_POINTER, e->time);
}

static
void handle_button_release(xcb_button_release_event_t *e) {
    NIL_LOG("mouse release %d", e->detail);
}

/** Mouse moved
 */
static
void handle_motion_notify(xcb_motion_notify_event_t *e) {
    NIL_LOG("mouse motion %d", e->detail);
}

/** Mouse entered
 */
static
void handle_enter_notify(xcb_enter_notify_event_t *e) {
    NIL_LOG("enter notify %d: event %d, child %d",
        e->detail, e->event, e->child);
}

/** Handler for creating a new window
 * xterm size request is 1x1?
 */
static
void handle_create_notify(xcb_create_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("create notify win=%d par=%d @ %d,%d %ux%u", e->window,
        e->parent, e->x, e->y, e->width, e->height);
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
    add_client(c, &nil_.ws[nil_.ws_idx]);
}

/** Handler for destroying a window
 */
static
void handle_destroy_notify(xcb_destroy_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("destroy notify win=%d", e->window);
    c = remove_client(e->window);
    if (c) {
        free(c);
    }
}

static
void handle_unmap_notify(xcb_unmap_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("unmap notify %d", e->window);
    c = find_client(e->window);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    c->map_state = XCB_MAP_STATE_UNMAPPED;
}

static
void handle_map_notify(xcb_map_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("map notify %d", e->window);
    c = find_client(e->window);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    c->map_state = XCB_MAP_STATE_VIEWABLE;
}

/** Handler for changing position, size
 */
static
void handle_configure_notify(xcb_configure_notify_event_t *e) {
    struct client_t *c;

    NIL_LOG("configure notify evt=%d win=%d abv=%d @ %d,%d %ux%u", e->event,
        e->window, e->above_sibling, e->x, e->y, e->width, e->height);

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
#endif
}

static
void handle_map_request(xcb_map_request_event_t *e) {
    xcb_get_window_attributes_reply_t *reply;
    struct client_t *c;

    NIL_LOG("map request win=%d", e->window);
    reply = xcb_get_window_attributes_reply(nil_.con,
        xcb_get_window_attributes(nil_.con, e->window), 0);
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
    c = find_client(e->window);
    if (!c) {
        NIL_ERR("no client %d", e->window);
        return;
    }
    arrange(&nil_.ws[nil_.ws_idx]);
    {   /* notify client size after rearrange */
        struct client_t *d;
        for (d = nil_.ws[nil_.ws_idx].client_first; d; d = d->next) {
            move_resize_client(d);      /* update window configuration */
        }
    }
    config_client(c);
    xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT,
        c->win, XCB_TIME_CURRENT_TIME);
    xcb_map_window(nil_.con, c->win);
    xcb_flush(nil_.con);
}

#define MAX_EVENTS_             (XCB_CONFIGURE_NOTIFY + 1)
typedef void (*event_handler_t)(xcb_generic_event_t *);
static const event_handler_t HANDLERS_[MAX_EVENTS_] = {
    [XCB_KEY_PRESS]         = (event_handler_t)&handle_key_press,
    [XCB_KEY_RELEASE]       = (event_handler_t)&handle_key_release,
    [XCB_BUTTON_PRESS]      = (event_handler_t)&handle_button_press,
    [XCB_BUTTON_RELEASE]    = (event_handler_t)&handle_button_release,
    [XCB_MOTION_NOTIFY]     = (event_handler_t)&handle_motion_notify,
    [XCB_ENTER_NOTIFY]      = (event_handler_t)&handle_enter_notify,
    [XCB_CREATE_NOTIFY]     = (event_handler_t)&handle_create_notify,
    [XCB_DESTROY_NOTIFY]    = (event_handler_t)&handle_destroy_notify,
    [XCB_UNMAP_NOTIFY]      = (event_handler_t)&handle_unmap_notify,
    [XCB_MAP_NOTIFY]        = (event_handler_t)&handle_map_notify,
    [XCB_MAP_REQUEST]       = (event_handler_t)&handle_map_request,
    [XCB_CONFIGURE_NOTIFY]  = (event_handler_t)&handle_configure_notify,
};

/** Events loop
 */
void recv_events() {
    xcb_generic_event_t *e;
    unsigned int type;
    while ((e = xcb_wait_for_event(nil_.con))) {
        type = e->response_type & ~0x80;
        if (type < MAX_EVENTS_ && HANDLERS_[type] != 0) {
            (*HANDLERS_[type])(e);
        } else {
            NIL_LOG("unknown type %u", type);
        }
        /* Free the Generic Event */
        free(e);
    }
}
