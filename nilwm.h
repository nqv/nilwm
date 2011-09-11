/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#ifndef NILWM_H_
#define NILWM_H_

#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_atom.h>

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

#define NIL_HAS_FLAG(x, f)      ((x) & (f))
#define NIL_SET_FLAG(x, f)      (x) |= (f)
#define NIL_CLEAR_FLAG(x, f)    (x) &= ~(f)

#ifdef __cplusplus
extern "C" {
#endif

enum {                          /* workspace layout type */
    LAYOUT_TILE = 0,
    LAYOUT_FREE
};

enum {                          /* client flags */
    CLIENT_HIDEN        = 1 << 1,
    CLIENT_FLOAT        = 1 << 2,
    CLIENT_FIXED        = 1 << 3,
};

struct client_t {
    xcb_window_t win;
    int16_t x, y;
    uint16_t w, h;
    uint16_t min_w, min_h;
    uint16_t max_w, max_h;
    uint16_t border_width;
    char *title;
    int map_state;
    unsigned int tags;
    unsigned int flags;
    struct client_t *prev, *next;
};

struct bar_t {
    xcb_window_t win;
    xcb_gcontext_t gc;
    int16_t x, y;
    uint16_t w, h;
};

struct font_t {
    xcb_font_t id;
    uint16_t height;
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
    uint16_t border_width;
    unsigned int border_color;
    unsigned int num_workspaces;
    uint16_t mod_key;
    struct key_t *keys;
    unsigned int keys_len;

    float mfact;    /* master */
    const char *font_name;
};

struct atom_t {
    xcb_atom_t net_supported;
    xcb_atom_t net_wm_name;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete;
    xcb_atom_t wm_state;
};

struct nilwm_t {
    xcb_connection_t *con;
    xcb_screen_t *scr;
    xcb_key_symbols_t *key_syms;
    uint16_t mask_numlock;
    uint16_t mask_capslock;
    uint16_t mask_shiftlock;
    uint16_t mask_modeswitch;
    int16_t x, y;
    uint16_t w, h;
    struct font_t font;
    struct atom_t atom;
    struct workspace_t *ws;
    unsigned int ws_idx;    /* current index of workspace */
};

/* client.c */
void init_client(struct client_t *self);
void config_client(struct client_t *self);
void check_client_size(struct client_t *self);
void move_resize_client(struct client_t *self);
void add_client(struct client_t *self, struct workspace_t *ws);
struct client_t *find_client(xcb_window_t win);
struct client_t *remove_client(xcb_window_t win, struct workspace_t **ws);

/* layout.c */
void arrange(struct workspace_t *self);

/* bar.c */
void draw_bar_text(struct bar_t *self, int x, int y, const char *str);

/* event.c */
void recv_events();

/* nilwm.c */
void spawn(const struct arg_t *arg);
int check_key(unsigned int mod, xcb_keysym_t key);
xcb_keysym_t get_keysym(xcb_keycode_t keycode, uint16_t state);
xcb_keycode_t get_keycode(xcb_keysym_t keysym);

/* global variables in nilwm.c */
extern struct nilwm_t nil_;
extern struct bar_t bar_;
extern const struct config_t cfg_;

#ifdef __cplusplus
}
#endif
#endif /* NILWM_H_ */
/* vim: set ts=4 sw=4 expandtab: */
