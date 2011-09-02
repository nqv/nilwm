/*
 * Nilwm - Lightweight X window manager.
 * See file LICENSE for license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include "nilwm.h"
#include "config.h"

static xcb_connection_t *con_;
static xcb_screen_t *scr_;

static
int init_screen() {
    /* get the first screen */
    scr_ = xcb_setup_roots_iterator(xcb_get_setup(con_)).data;
    NIL_LOG("screen %d (%dx%d)", scr_->root,
            scr_->width_in_pixels, scr_->height_in_pixels);

    /* TODO: set up all existing windows */
    return 0;
}

static
int init_key() {
    return 0;
}

static
int init_mouse() {
    xcb_grab_button(con_, 0, scr_->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scr_->root,
        XCB_NONE, 1 /* left mouse button */, MODKEY);
    xcb_grab_button(con_, 0, scr_->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scr_->root,
        XCB_NONE, 2 /* middle mouse button */, MODKEY);
    xcb_grab_button(con_, 0, scr_->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scr_->root,
        XCB_NONE, 3 /* right mouse button */, MODKEY);
    return 0;
}

static
void destroy() {
    xcb_flush(con_);
    xcb_disconnect(con_);
}

static
void handle_key_press(xcb_key_press_event_t *e) {
    NIL_LOG("key press %d", e->detail);
}

static
void handle_key_release(xcb_key_release_event_t *e) {
    NIL_LOG("key releas %d", e->detail);
}

/**
 * Handle the ButtonPress event
 */
static
void handle_button_press(xcb_button_press_event_t *e) {
    NIL_LOG("mouse %d pressed: event %d, child %d (%d,%d)",
        e->detail, e->event, e->child, e->event_x, e->event_y);
}

static
void handle_button_release(xcb_button_release_event_t *e) {
    NIL_LOG("button releas %d", e->detail);
}

static
void handle_motion_notify(xcb_motion_notify_event_t *e) {
    NIL_LOG("button releas %d", e->detail);
}

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
void handle_map_request(xcb_map_request_event_t *e) {
    NIL_LOG("map request %d", e->window);
}

/**
 * Events loop
 */
static
void recv_events() {
    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(con_))) {
        switch (e->response_type & ~0x80) {
        case XCB_KEY_PRESS:
            handle_key_press((xcb_key_press_event_t *)e);
            break;
        case XCB_KEY_RELEASE:
            handle_key_release((xcb_key_release_event_t *)e);
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
        case XCB_MAP_REQUEST:
            handle_map_request((xcb_map_request_event_t *)e);
            break;
        default:
            /* Unknown event type, ignore it */
            break;
        }
        /* Free the Generic Event */
        free(e);
    }
}

int main(int argc, char **argv) {
    /* open connection with the server */
    con_ = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(con_)) {
        fprintf(stderr, "xcb_connect\n");
        exit(1);
    }
    if ((init_screen() < 0) || (init_key() < 0) || (init_mouse() < 0)) {
        xcb_disconnect(con_);
        exit(1);
    }
    recv_events();
    destroy();
    return 0;
}
