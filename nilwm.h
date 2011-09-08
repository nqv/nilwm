/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#ifndef NILWM_H_
#define NILWM_H_

#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#define NIL_QUOTE(x)            #x
#define NIL_TOSTR(x)            NIL_QUOTE(x)
#define NIL_SRC                 __FILE__ ":" NIL_TOSTR(__LINE__)
#define NIL_ERR(fmt, ...)       fprintf(stderr, "*" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
#ifdef DEBUG
# define NIL_LOG(fmt, ...)      fprintf(stdout, "-" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
#else
# define NIL_LOG(fmt, ...)
#endif
#define NIL_LEN(x)              (sizeof(x) / sizeof((x)[0]))

#ifdef __cplusplus
extern "C" {
#endif

enum {
    LAYOUT_TILE = 0,
    LAYOUT_FREE
};

enum {
    CLIENT_FLOAT    = 0x01,
};

struct client_t {
    char *title;
    int16_t x, y;
    uint16_t w, h;
    uint16_t min_w, min_h;
    uint16_t max_w, max_h;
    uint16_t border_width;
    unsigned int tags;
    unsigned int flags;
    int map_state;
    xcb_window_t win;
    struct client_t *prev, *next;
};

struct arg_t {
    int c;
    void *v;
};

struct key_t {
    unsigned int mod;
    unsigned int keysym;
    void (*func)(const struct arg_t *arg);
    struct arg_t arg;
};

struct workspace_t {
    struct client_t *client_first;
    struct client_t *client_last;
    int layout;
};

struct config_t {
    unsigned int border_width;
    unsigned int border_color;
    unsigned int num_workspaces;
    unsigned int mod_key;
    struct key_t *keys;
    unsigned int keys_len;

    float mfact;    /* master */
};

struct nilwm_t {
    xcb_connection_t *con;
    xcb_screen_t *scr;
    xcb_key_symbols_t *key_syms;
    uint16_t mask_numlock;
    uint16_t mask_capslock;
    uint16_t mask_shiftlock;
    uint16_t mask_modeswitch;
    struct workspace_t *ws;
    unsigned int ws_idx;    /* current index of workspace */
};

/* client.c */
void init_client(struct client_t *self);
void config_client(struct client_t *self);
void move_resize_client(struct client_t *self);
void add_client(struct client_t *self, struct workspace_t *ws);
struct client_t *find_client(xcb_window_t win);
struct client_t *remove_client(xcb_window_t win);

/* layout.c */
void arrange(struct workspace_t *ws);

/* event.c */
void recv_events();

/* nilwm.c */
void spawn(const struct arg_t *arg);
int check_key(unsigned int mod, xcb_keysym_t key);
xcb_keysym_t get_keysym(xcb_keycode_t keycode, uint16_t state);
xcb_keycode_t get_keycode(xcb_keysym_t keysym);

/* global variables in nilwm.c */
extern struct nilwm_t nil_;
extern const struct config_t cfg_;

#ifdef __cplusplus
}
#endif
#endif /* NILWM_H_ */
