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

static
void handle_unmap_notify(xcb_unmap_notify_event_t *e) {
    NIL_LOG("unmap notify %d", e->window);
}

static
void handle_destroy_notify(xcb_destroy_notify_event_t *e) {
    NIL_LOG("destroy notify %d", e->window);
}

static
void handle_map_request(xcb_map_request_event_t *e) {
    xcb_get_window_attributes_reply_t *reply;
    struct client_t *client;

    NIL_LOG("map request %d", e->window);

    reply = xcb_get_window_attributes_reply(nil_.con,
        xcb_get_window_attributes(nil_.con, e->window), NULL);
    if (!reply) {
        NIL_ERR("no reply %d", e->window);
        return;
    }
    if (reply->override_redirect) {
        NIL_ERR("override_redirect %d", e->window);
        goto end;
    }
    client = malloc(sizeof(struct client_t));
    if (!client) {
        NIL_ERR("out of mem %d", e->window);
        goto end;
    }
    client->win = e->window;
    init_client(client);
    add_client(client);
    xcb_map_window(nil_.con, client->win);
end:
    free(reply);
}

/** Events loop
 */
void recv_events() {
    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(nil_.con))) {
        switch (e->response_type & ~0x80) {
        case XCB_KEY_PRESS:
            handle_key_press((xcb_key_press_event_t *)e);
            break;
        case XCB_BUTTON_PRESS:
            handle_button_press((xcb_button_press_event_t *)e);
            break;
        case XCB_BUTTON_RELEASE:
            handle_button_release((xcb_button_release_event_t *)e);
            break;
        case XCB_MOTION_NOTIFY:
            handle_motion_notify((xcb_motion_notify_event_t *)e);
            break;
        case XCB_ENTER_NOTIFY:
            handle_enter_notify((xcb_enter_notify_event_t *)e);
            break;
        case XCB_UNMAP_NOTIFY:
            handle_unmap_notify((xcb_unmap_notify_event_t *)e);
            break;
        case XCB_DESTROY_NOTIFY:
            handle_destroy_notify((xcb_destroy_notify_event_t *)e);
            break;
        case XCB_MAP_REQUEST:
            handle_map_request((xcb_map_request_event_t *)e);
            break;
        default:                /* Unknown event type, ignore it */
            NIL_LOG("unknown type 0x%x", e->response_type & ~0x80);
            break;
        }
        /* Free the Generic Event */
        free(e);
    }
}
