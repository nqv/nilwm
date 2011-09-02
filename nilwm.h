/*
 * Nilwm - Lightweight X window manager.
 * See file LICENSE for license information.
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

struct client_t {
    char *title;
    int x, y;
    unsigned int w, h;
    unsigned int min_w, min_h;
    unsigned int max_w, max_h;
    struct client_t *prev, *next;
};

struct config_t {
    unsigned int border_color;
};

#endif /* NILWM_H_ */
