/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#ifndef NILWM_H_
#define NILWM_H_

#define NIL_QUOTE(x)            #x
#define NIL_TOSTR(x)            NIL_QUOTE(x)
#define NIL_SRC                 __FILE__ ":" NIL_TOSTR(__LINE__)
#define NIL_ERR(fmt, ...)       fprintf(stderr, "-" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
#ifdef DEBUG
# define NIL_LOG(fmt, ...)      fprintf(stdout, "*" NIL_SRC "\t\t" fmt "\n", __VA_ARGS__)
#else
# define NIL_LOG(fmt, ...)
#endif
#define NIL_LEN(x)              (sizeof(x) / sizeof((x)[0]))

#ifdef __cplusplus
extern "C" {
#endif

struct client_t {
    char *title;
    int x, y;
    unsigned int w, h;
    unsigned int min_w, min_h;
    unsigned int max_w, max_h;
    unsigned int tags;
    unsigned int flags;
    xcb_window_t win;
    struct client_t *prev, *next;
};

struct arg_t {
    int c;
    void *v;
};

struct key_t {
    unsigned int mod;
    xcb_keycode_t key;
    void (*func)(const struct arg_t *arg);
    struct arg_t arg;
};

struct config_t {
    unsigned int border_width;
    unsigned int border_color;
};

struct nilwm_t {
    xcb_connection_t *con;
    xcb_screen_t *scr;
    struct client_t *client_list;
    struct config_t *cfg;
};

/* client.c */
void init_client(struct client_t *self);
void add_client(struct client_t *self);

/* event.c */
void recv_events();

/* nilwm.c */
void spawn(const struct arg_t *arg);
int check_key(unsigned int mod, xcb_keycode_t key);

/* global variables in nilwm.c */
extern struct nilwm_t nil_;

#ifdef __cplusplus
}
#endif
#endif /* NILWM_H_ */
