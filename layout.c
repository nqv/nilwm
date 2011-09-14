/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include "nilwm.h"

#define RSZ_CLIENT_(C, X, Y, W, H)  C->x = X; C->y = Y; \
    C->w = (W) - 2 * C->border_width; C->h = (H) - 2 * C->border_width
#define CAN_TILE_(C)     (NIL_HAS_FLAG(C->flags, CLIENT_MAPPED) \
    && !NIL_HAS_FLAG(C->flags, CLIENT_FLOAT))

struct layout_handler_t {
    void (*arrange)(struct workspace_t *);
    void (*focus)(struct workspace_t *, int nav);
    void (*swap)(struct workspace_t *, int nav);
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
    for (m = self->first; m; m = m->next) {
        if (CAN_TILE_(m)) {
            break;
        }
    }
    if (!m) {
        return;
    }
    /* next client with is not float */
    for (c = m->next; c; c = c->next) {
        if (CAN_TILE_(c)) {
            break;
        }
    }
    if (!c) {           /* 1 window */
        RSZ_CLIENT_(m, nil_.x, nil_.y, nil_.w, nil_.h);
        return;
    }
    w = cfg_.mfact * nil_.w;
    RSZ_CLIENT_(m, nil_.x, nil_.y, w, nil_.h);
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
            RSZ_CLIENT_(c, x, y, w, nil_.h + nil_.y - y);
            break;
        }
        RSZ_CLIENT_(c, x, y, w, h);
        y = c->y + c->h + 2 * c->border_width;
        --n;
    } while (0 != (c = c->next));
}

/** Focus next window in tile mode
 */
static
void focus_tile(struct workspace_t *self, int nav) {
    struct client_t *c;

    if (!self->focus || nav == NAV_MASTER) {
        if (self->first) {          /* focus first window if */
            xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT,
                self->first->win, XCB_CURRENT_TIME);
        }
        return;
    }
    if (nav == NAV_NEXT) {
        c = self->focus->next;
        if (c) {
            NIL_LOG("focus next %d", c->win);
            xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT,
                c->win, XCB_CURRENT_TIME);
        }
        return;
    }
    if (nav == NAV_PREV) {

    }
}

static
void arrange_free(struct workspace_t *self) {

}

/** Handlers for each type of layouts
 */
static const struct layout_handler_t layouts_[] = {
    [LAYOUT_TILE] = {
        .arrange = &arrange_tile,
        .focus = &focus_tile
    },
    [LAYOUT_FREE] = {
        .arrange = &arrange_free
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

void change_focus(int nav) {
    const struct layout_handler_t *h;

    NIL_LOG("focus %d", nil_.ws_idx);
    h = &layouts_[nil_.ws[nil_.ws_idx].layout];
    if (h->focus) {
        (*h->focus)(&nil_.ws[nil_.ws_idx], nav);
    }
}

/* vim: set ts=4 sw=4 expandtab: */
