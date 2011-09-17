/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include "nilwm.h"

#define RESIZE_CLIENT_(C, X, Y, W, H)       \
    C->x = X; C->y = Y;                     \
    C->w = (W) - 2 * C->border_width; C->h = (H) - 2 * C->border_width
#define NEXT_CLIENT_(C, FROM, COND)         \
    for (C = FROM; C; C = C->next) {        \
        if (COND) { break; }                \
    }
#define PREV_CLIENT_(C, FROM, COND)         \
    for (C = FROM; C; C = C->prev) {        \
        if (COND) { break; }                \
    }
#define CAN_TILE_(C)     (NIL_HAS_FLAG(C->flags, CLIENT_MAPPED) \
    && !NIL_HAS_FLAG(C->flags, CLIENT_FLOAT))

struct layout_handler_t {
    void (*arrange)(struct workspace_t *);
    void (*focus)(struct workspace_t *, int dir);
    void (*swap)(struct workspace_t *, int dir);
};

/** Tile windows
 * Not arrange floating window
 */
static
void arrange_tile(struct workspace_t *self) {
    struct client_t *m, *c;
    int16_t x, y;
    uint16_t w, h;
    unsigned int n;

    /* master */
    NEXT_CLIENT_(m, self->first, CAN_TILE_(m));
    if (!m) {
        return;
    }
    /* next client with is not float */
    NEXT_CLIENT_(c, m->next, CAN_TILE_(c));
    if (!c) {           /* 1 window */
        RESIZE_CLIENT_(m, nil_.x, nil_.y, nil_.w, nil_.h);
        return;
    }
    w = cfg_.mfact * nil_.w;
    RESIZE_CLIENT_(m, nil_.x, nil_.y, w, nil_.h);
    /* next position */
    x = (int)w;
    y = nil_.y;
    w = nil_.w - w;
    /* get number of clients */
    n = 1;  /* having at least 1 */
    for (m = c->next; m; m = m->next) {
        if (CAN_TILE_(m)) {
            ++n;
        }
    }
    h = nil_.h / n;
    do {
        if (!CAN_TILE_(c)) {
            continue;
        }
        if (n == 1) {       /* last one */
            RESIZE_CLIENT_(c, x, y, w, nil_.h + nil_.y - y);
            break;
        }
        RESIZE_CLIENT_(c, x, y, w, h);
        y = c->y + c->h + 2 * c->border_width;
        --n;
    } while (0 != (c = c->next));
}

/** Focus next window in tile mode
 */
static
void focus_tile(struct workspace_t *self, int dir) {
    struct client_t *c;

    if (!self->focus) {
        /* focus first window if no window focused */
        NEXT_CLIENT_(c, self->first, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
    } else if (dir == NAV_NEXT) {
        NEXT_CLIENT_(c, self->focus->next, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
        if (!c) {
            c = self->first;    /* loop */
        }
    } else if (dir == NAV_PREV) {
        PREV_CLIENT_(c, self->focus->prev, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
        if (!c) {
            c = self->last;     /* loop */
        }
    } else {
        NEXT_CLIENT_(c, self->first, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
    }
    if (c) {
        xcb_warp_pointer(nil_.con, XCB_NONE, c->win, 0, 0, 0, 0, 0, 0);
        xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT, c->win,
            XCB_CURRENT_TIME);
    }
}

static
void swap_tile(struct workspace_t *self, int dir) {
    struct client_t *c;

    if (!self->focus) {
        return;
    }
    if (dir == NAV_NEXT) {
        NEXT_CLIENT_(c, self->focus->next, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
        if (!c) {
            c = self->first;    /* loop */
        }
    } else if (dir == NAV_PREV) {
        PREV_CLIENT_(c, self->focus->prev, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
        if (!c) {
            c = self->last;     /* loop */
        }
    } else {
        NEXT_CLIENT_(c, self->first, NIL_HAS_FLAG(c->flags, CLIENT_MAPPED));
    }
    if (c && c != self->focus) {
        swap_client(self->focus, c);
    }
}

/** Handlers for each type of layouts
 */
static const struct layout_handler_t layouts_[] = {
    [LAYOUT_TILE] = {
        .arrange    = &arrange_tile,
        .focus      = &focus_tile,
        .swap       = &swap_tile,
    },
    [LAYOUT_FREE] = {
        .arrange    = 0,
        .focus      = &focus_tile,
        .swap       = &swap_tile,
    },
};

void arrange() {
    const struct layout_handler_t *h;

    NIL_LOG("arrange %d", nil_.ws_idx);
    h = &layouts_[nil_.ws[nil_.ws_idx].layout];
    if (h->arrange) {
        (*h->arrange)(&nil_.ws[nil_.ws_idx]);
    }
}

void change_focus(int dir) {
    const struct layout_handler_t *h;

    NIL_LOG("focus %d", nil_.ws_idx);
    h = &layouts_[nil_.ws[nil_.ws_idx].layout];
    if (h->focus) {
        (*h->focus)(&nil_.ws[nil_.ws_idx], dir);
    }
}

/* vim: set ts=4 sw=4 expandtab: */
