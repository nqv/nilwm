/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include "nilwm.h"
#include "config.h"

struct nilwm_t nil_;

void spawn(const struct arg_t *arg) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {   /* child process */
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        NIL_ERR("execvp %s", ((char **)arg->v)[0]);
        exit(1);
    } else if (pid < 0) {
        NIL_ERR("fork %d", pid);
    }
}

int check_shortcut(unsigned int mod, xcb_keycode_t key) {
    unsigned int i;
    const struct shortcut_t *k;

    for (i = 0; i < NIL_LEN(SHORTCUTS); ++i) {
        k = &SHORTCUTS[i];
        if (k->mod == mod && k->key == key) {
            (*k->func)(&k->arg);
            return 1;
        }
    }
    return 0;
}

static
int init_screen() {
    uint32_t values;
    xcb_void_cookie_t cookie;

    /* get the first screen */
    nil_.scr = xcb_setup_roots_iterator(xcb_get_setup(nil_.con)).data;
    NIL_LOG("screen %d (%dx%d)", nil_.scr->root,
            nil_.scr->width_in_pixels, nil_.scr->height_in_pixels);

    /* Select for events, and at the same time, send SubstructureRedirect */
    values = XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    cookie = xcb_change_window_attributes_checked(nil_.con, nil_.scr->root,
        XCB_CW_EVENT_MASK, &values);
    if (xcb_request_check(nil_.con, cookie)) {
        NIL_ERR("Another window manager is already running %u", cookie.sequence);
        return 1;
    }
    /* TODO: set up all existing windows */
    nil_.client_list = 0;
    return 0;
}

static
int init_key() {
    return 0;
}

static
int init_mouse() {
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, 1 /* left mouse button */, MODKEY);
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, 2 /* middle mouse button */, MODKEY);
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, 3 /* right mouse button */, MODKEY);
    return 0;
}

static
void cleanup() {
    xcb_flush(nil_.con);
    xcb_disconnect(nil_.con);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    /* open connection with the server */
    nil_.con = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(nil_.con)) {
        fprintf(stderr, "xcb_connect\n");
        exit(1);
    }
    if ((init_screen() < 0) || (init_key() < 0) || (init_mouse() < 0)) {
        xcb_disconnect(nil_.con);
        exit(1);
    }
    recv_events();
    cleanup();
    return 0;
}
