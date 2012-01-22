/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#ifndef NILWM_H_
#define NILWM_H_

#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

#define NIL_QUOTE(x)            #x
#define NIL_TOSTR(x)            NIL_QUOTE(x)
#define NIL_SRC                 __FILE__ ":" NIL_TOSTR(__LINE__)
#ifdef DEBUG
# define NIL_ERR(fmt, ...)      fprintf(stderr, "*" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
# define NIL_LOG(fmt, ...)      fprintf(stdout, "-" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
#else
# define NIL_ERR(fmt, ...)      fprintf(stderr, fmt "\n", __VA_ARGS__)
# define NIL_LOG(fmt, ...)
#endif
#define NIL_INLINE              inline __attribute__((always_inline))
#define NIL_UNUSED(x)           _ ## x __attribute__((unused))
#define NIL_LEN(x)              (sizeof(x) / sizeof((x)[0]))

#define NIL_HAS_FLAG(x, f)      ((x) & (f))
#define NIL_SET_FLAG(x, f)      (x) |= (f)
#define NIL_CLEAR_FLAG(x, f)    (x) &= ~(f)

#ifdef __cplusplus
extern "C" {
#endif

enum {                              /* workspace layout type */
    LAYOUT_TILE         = 0,
    LAYOUT_FREE,
    NUM_LAYOUT,
};

enum {                              /* client flags */
    CLIENT_DISPLAY      = 1 << 0,   /* displayed */
    CLIENT_MAPPED       = 1 << 1,
    CLIENT_FLOAT        = 1 << 2,   /* float mode */
    CLIENT_FIXED        = 1 << 3,   /* size fixed (min = max) */
    CLIENT_FOCUS        = 1 << 4,   /* already focused */
};

enum {                              /* for focus/swap */
    NAV_PREV            = -1,
    NAV_MASTER          = 0,
    NAV_NEXT            = 1,
};

enum {
    BAR_WS              = 0,
    BAR_SYM,
    BAR_TASK,
    BAR_STATUS,
    BAR_ICON,
    NUM_BAR,
};

enum {                              /* box flags */
    BOX_LEFT            = 0 << 0,   /* 2 bits for box floating */
    BOX_RIGHT           = 1 << 0,
    BOX_FIXED           = 2 << 0,
    BOX_TEXT_LEFT       = 0 << 2,   /* 2 bits for text alignment */
    BOX_TEXT_RIGHT      = 1 << 2,
    BOX_TEXT_CENTER     = 2 << 2,
};

enum {
    CURSOR_NORMAL       = 0,
    CURSOR_MOVE,
    CURSOR_RESIZE,
    NUM_CURSOR,
};

/* window wrapper */
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
    struct client_t *next;
    struct client_t **prev;
};

struct bar_box_t {
    int16_t x;
    uint16_t w;
    void (*click)(struct bar_box_t *self, int x);
    unsigned int flags;
};

/* info bar */
struct bar_t {
    xcb_window_t win;
    xcb_gcontext_t gc;
    int16_t x, y;
    uint16_t w, h;
    struct bar_box_t box[NUM_BAR];
};

struct mouse_event_t {
    int mode;   /* is also CURSOR */
    struct client_t *client;
    struct workspace_t *ws;
    int16_t x1, y1;
    int16_t x2, y2;
};

/* font information */
struct font_t {
    xcb_font_t id;
    uint16_t ascent;
    uint16_t descent;
};

/* colors from user's configuration */
struct color_t {
    uint32_t border;
    uint32_t focus;
    uint32_t bar_bg;
    uint32_t bar_fg;
    uint32_t bar_sel;
    uint32_t bar_occ;
    uint32_t bar_urg;
};

struct arg_t {
    union {
        int i;
        unsigned int u;
        void *v;
    };
};

struct key_t {
    unsigned int mod;
    unsigned int keysym;
    void (*func)(const struct arg_t *arg);
    struct arg_t arg;
};

struct workspace_t {
    struct client_t *first;
    struct client_t *last;
    struct client_t *focus;
    int layout;
    int master_size;
};

struct layout_t {
    const char *symbol;
    void (*arrange)(struct workspace_t *);
    void (*focus)(struct workspace_t *, int dir);
    void (*swap)(struct workspace_t *, int dir);
    void (*move)(struct workspace_t *, struct mouse_event_t *e);
    void (*resize)(struct workspace_t *, struct mouse_event_t *e);
};

struct config_t {
    uint16_t border_width;
    unsigned int num_workspaces;
    uint16_t mod_key;
    struct key_t *keys;
    unsigned int keys_len;

    unsigned int master_size;       /* master factor */
    const char *font_name;

    const char *border_color;
    const char *focus_color;
    const char *bar_bg_color;
    const char *bar_fg_color;
    const char *bar_sel_color;
    const char *bar_occ_color;
    const char *bar_urg_color;
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
    xcb_cursor_t cursor[NUM_CURSOR];
    uint16_t mask_numlock;
    uint16_t mask_capslock;
    uint16_t mask_shiftlock;
    uint16_t mask_modeswitch;
    int16_t x, y;
    uint16_t w, h;
    struct color_t color;
    struct font_t font;
    struct atom_t atom;
    struct workspace_t *ws;
    unsigned int ws_idx;    /* current index of workspace */
};

/* client.c */
void init_client(struct client_t *self);
void config_client(struct client_t *self);
int check_client_size(struct client_t *self);
void update_client_geom(struct client_t *self);
void attach_client(struct client_t *self, struct workspace_t *ws);
void detach_client(struct client_t *self);
struct client_t *find_client(xcb_window_t win, struct workspace_t **ws);
void focus_client(struct client_t *self);
void blur_client(struct client_t *self);
void raise_client(struct client_t *self);
void swap_client(struct client_t *self, struct client_t *c);
void hide_client(struct client_t *self);
void show_client(struct client_t *self);

/* layout.c */
const struct layout_t *get_layout(struct workspace_t *self);
void arrange_ws(struct workspace_t *self);
void hide_ws(struct workspace_t *self);
void show_ws(struct workspace_t *self);

/* bar.c */
void config_bar();
void text_bar(int x, int y, const char *str);
int click_bar(int x);
void update_bar_ws(unsigned int idx);
void update_bar_sym();
void update_bar_status();

/* event.c */
void recv_events();

/* nilwm.c */
void spawn(const struct arg_t *arg);
void focus(const struct arg_t *arg);
void swap(const struct arg_t *arg);
void kill_focused(const struct arg_t *arg);
void toggle_floating(const struct arg_t *arg);
void set_msize(const struct arg_t *arg);
void set_layout(const struct arg_t *arg);
void change_ws(const struct arg_t *arg);
void push(const struct arg_t *arg);
void quit(const struct arg_t *arg);

int get_text_prop(xcb_window_t win, xcb_atom_t atom, char *s, unsigned int len);
int cal_text_width(const char *text, int len);
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
