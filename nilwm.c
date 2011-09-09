/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include "nilwm.h"

struct nilwm_t nil_;

static
void handle_sigchld() {
    while (0 < waitpid(-1, 0, WNOHANG)) {
    }
}

void spawn(const struct arg_t *arg) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {   /* child process */
        close(xcb_get_file_descriptor(nil_.con));
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        NIL_ERR("execvp %s", ((char **)arg->v)[0]);
        exit(1);
    } else if (pid < 0) {
        NIL_ERR("fork %d", pid);
    }
}

int check_key(unsigned int mod, xcb_keysym_t key) {
    unsigned int i;
    const struct key_t *k;

    for (i = 0; i < cfg_.keys_len; ++i) {
        k = &cfg_.keys[i];
        if (k->mod == mod && k->keysym == key) {
            (*k->func)(&k->arg);
            return 1;
        }
    }
    return 0;
}

/** Get KeySymbol from a KeyCode according to its state
 */
xcb_keysym_t get_keysym(xcb_keycode_t keycode, uint16_t state) {
    xcb_keysym_t k0, k1;

    /* Mode_Switch is ON */
    if (state & nil_.mask_modeswitch) {
        k0 = xcb_key_symbols_get_keysym(nil_.key_syms, keycode, 2);
        k1 = xcb_key_symbols_get_keysym(nil_.key_syms, keycode, 3);
    } else {
        k0 = xcb_key_symbols_get_keysym(nil_.key_syms, keycode, 0);
        k1 = xcb_key_symbols_get_keysym(nil_.key_syms, keycode, 1);
    }
    if (k1 == XCB_NO_SYMBOL) {
        k1 = k0;
    }
    /* NUM on and is from keypad */
    if ((state & nil_.mask_numlock) && xcb_is_keypad_key(k1)) {
        if ((state & XCB_MOD_MASK_SHIFT)
            || ((state & XCB_MOD_MASK_LOCK) && (state & nil_.mask_shiftlock))) {
            return k0;
        } else {
            return k1;
        }
    }
    if (!(state & XCB_MOD_MASK_SHIFT)) {
        if (!(state & XCB_MOD_MASK_LOCK)) {         /* SHIFT off, CAPS off */
            return k0;
        } else if (state & nil_.mask_capslock) {    /* SHIFT off, CAPS on */
            return k1;
        }
    } else {
        return k1;
    }
    NIL_ERR("no symbol: state=0x%x keycode=0x%x", state, keycode);
    return XCB_NO_SYMBOL;
}

/** Get the first keycode
 */
xcb_keycode_t get_keycode(xcb_keysym_t keysym) {
    xcb_keycode_t k, *pk;

    /* only use the first one */
    pk = xcb_key_symbols_get_keycode(nil_.key_syms, keysym);
    if (pk == 0) {
        NIL_ERR("no keycode: 0x%x", keysym);
        return 0;
    }
    k = *pk;
    free(pk);
    return k;
}

static
void update_keys_mask() {
    xcb_keycode_t key_num, key_shift, key_caps, key_mode, key;
    xcb_get_modifier_mapping_reply_t *reply;
    xcb_keycode_t *codes;
    unsigned int i, j;

    nil_.mask_numlock    = 0;
    nil_.mask_shiftlock  = 0;
    nil_.mask_capslock   = 0;
    nil_.mask_modeswitch = 0;
    key_num   = get_keycode(XK_Num_Lock);
    key_shift = get_keycode(XK_Shift_Lock);
    key_caps  = get_keycode(XK_Caps_Lock);
    key_mode  = get_keycode(XK_Mode_switch);

    reply = xcb_get_modifier_mapping_reply(nil_.con,
        xcb_get_modifier_mapping_unchecked(nil_.con), NULL);
    codes = xcb_get_modifier_mapping_keycodes(reply);

    /* The number of keycodes in the list is 8 * keycodes_per_modifier */
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < reply->keycodes_per_modifier; ++j) {
            key = codes[i * reply->keycodes_per_modifier + j];
            if (!key) {
                continue;
            }
            if (key == key_num) {
                nil_.mask_numlock = (uint16_t)(1 << i);
            }
            if (key == key_shift) {
                nil_.mask_shiftlock = (uint16_t)(1 << i);
            }
            if (key == key_caps) {
                nil_.mask_capslock = (uint16_t)(1 << i);
            }
            if (key == key_mode) {
                nil_.mask_modeswitch = (uint16_t)(1 << i);
            }
        }
    }
    NIL_LOG("mask num=0x%x shift=0x%x caps=0x%x mode=0x%x", nil_.mask_numlock,
        nil_.mask_shiftlock, nil_.mask_capslock, nil_.mask_modeswitch);
    free(reply);
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
    values = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_PROPERTY_CHANGE
        | XCB_EVENT_MASK_EXPOSURE;

    cookie = xcb_change_window_attributes_checked(nil_.con, nil_.scr->root,
        XCB_CW_EVENT_MASK, &values);
    if (xcb_request_check(nil_.con, cookie)) {
        NIL_ERR("Another window manager is already running %u", cookie.sequence);
        return 1;
    }
    /* TODO: set up all existing windows */
    return 0;
}

static
int init_key() {
    unsigned int i;
    const struct key_t *k;
    xcb_keycode_t key;

    nil_.key_syms = xcb_key_symbols_alloc(nil_.con);
    update_keys_mask();

    xcb_ungrab_key(nil_.con, XCB_GRAB_ANY, nil_.scr->root, XCB_MOD_MASK_ANY);
    for (i = 0; i < cfg_.keys_len; ++i) {
        k = &cfg_.keys[i];
        key = get_keycode(k->keysym);
        if (key == 0) {
            continue;
        }
        xcb_grab_key(nil_.con, 1, nil_.scr->root, k->mod, key,
             XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
    return 0;
}

/**
 * Grab mouse buttons.
 */
static
int init_mouse() {
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, XCB_BUTTON_INDEX_1 /* left mouse button */, cfg_.mod_key);
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, XCB_BUTTON_INDEX_2 /* middle mouse button */, cfg_.mod_key);
    xcb_grab_button(nil_.con, 0, nil_.scr->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, nil_.scr->root,
        XCB_NONE, XCB_BUTTON_INDEX_3 /* right mouse button */, cfg_.mod_key);
    return 0;
}

static
int init_wm() {
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = &handle_sigchld;
    sigaction(SIGCHLD, &act, 0);

    nil_.ws = malloc(sizeof(struct workspace_t) * cfg_.num_workspaces);
    if (!nil_.ws) {
        NIL_ERR("out of mem %d", cfg_.num_workspaces);
        return -1;
    }
    memset(nil_.ws, 0, sizeof(struct workspace_t) * cfg_.num_workspaces);
    nil_.ws_idx = 0;
    return 0;
}

static
void cleanup() {
    if (nil_.key_syms) {
        xcb_key_symbols_free(nil_.key_syms);
    }
    xcb_ungrab_keyboard(nil_.con, XCB_TIME_CURRENT_TIME);
    xcb_destroy_subwindows(nil_.con, nil_.scr->root);
    xcb_flush(nil_.con);
    xcb_disconnect(nil_.con);
    if (nil_.ws) {
        free(nil_.ws);
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* open connection with the server */
    nil_.con = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(nil_.con)) {
        NIL_ERR("xcb_connect %p", (void *)nil_.con);
        exit(1);
    }
    if ((init_screen() < 0) || (init_key() < 0) || (init_mouse() < 0)) {
        xcb_disconnect(nil_.con);
        exit(1);
    }
    if (init_wm() < 0) {
        cleanup();
        exit(1);
    }
    xcb_flush(nil_.con);
    recv_events();
    cleanup();
    return 0;
}
