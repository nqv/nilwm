/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include "nilwm.h"

#define SYMBOL_TILE_            "T"
#define SYMBOL_FREE_            "F"

#define RESIZE_CLIENT_(C, X, Y, W, H)       \
    C->x = X; C->y = Y;                     \
    C->w = (W) - 2 * C->border_width; C->h = (H) - 2 * C->border_width
#define NEXT_CLIENT_(C, FROM, COND)         \
    for (C = FROM; C; C = C->next) {        \
        if (COND) { break; }                \
    }
#define PREV_CLIENT_(C, FROM, COND)         \
    for (C = FROM; C != *C->prev; C = *C->prev) {   \
        if (COND) { break; }                \
    }
#define EACH_CLIENT_(WS, C, CMD)            \
    C = WS->first;                          \
    while (C) {                             \
        CMD(C);                             \
        C = C->next;                        \
    }
#define CAN_TILE_(C)     (NIL_HAS_FLAG(C->flags, CLIENT_DISPLAY)   \
    && !NIL_HAS_FLAG(C->flags, CLIENT_FLOAT))
#define CAN_FOCUS_(C)    (NIL_HAS_FLAG(C->flags, CLIENT_DISPLAY))

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
        update_client_geom(m);
        return;
    }
    w = self->master_size / 100.0 * nil_.w;
    RESIZE_CLIENT_(m, nil_.x, nil_.y, w, nil_.h);
    update_client_geom(m);
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
    NIL_LOG("tile n=%d h=%u", n, h);
    do {
        if (!CAN_TILE_(c)) {
            continue;
        }
        if (n == 1) {       /* last one */
            RESIZE_CLIENT_(c, x, y, w, nil_.h + nil_.y - y);
            update_client_geom(c);
            break;
        }
        RESIZE_CLIENT_(c, x, y, w, h);
        update_client_geom(c);
        y = c->y + c->h + 2 * c->border_width;
        --n;
    } while (0 != (c = c->next));
}

/** Focus next window in tile mode
 */
static
void focus_tile(struct workspace_t *self, const int dir) {
    struct client_t *c;

    if (!self->focus) {
        /* focus first window if no window focused */
        NEXT_CLIENT_(c, self->first, CAN_FOCUS_(c));
    } else if (dir == NAV_NEXT) {
        NEXT_CLIENT_(c, self->focus->next, CAN_FOCUS_(c));
        if (!c) {       /* loop */
            for (c = self->first; c && (c != self->focus); c = c->next) {
                if (CAN_FOCUS_(c)) {
                    break;
                }
            }
        }
    } else if (dir == NAV_PREV) {
        PREV_CLIENT_(c, *self->focus->prev, CAN_FOCUS_(c));
        if (!c) {       /* loop */
            for (c = self->last; (c != *c->prev) && (c != self->focus); c = *c->prev) {
                if (CAN_FOCUS_(c)) {
                    break;
                }
            }
        }
    } else {
        NEXT_CLIENT_(c, self->first, CAN_FOCUS_(c));
    }
    if (c) {
        xcb_set_input_focus(nil_.con, XCB_INPUT_FOCUS_POINTER_ROOT, c->win,
            XCB_CURRENT_TIME);
        raise_client(c);
    }
}

/** Only swap with tiled windows
 */
static
void swap_tile(struct workspace_t *self, const int dir) {
    struct client_t *c;

    if (!self->focus) {
        return;
    }
    if (dir == NAV_NEXT) {
        NEXT_CLIENT_(c, self->focus->next, CAN_TILE_(c));
        if (!c) {   /* loop */
            for (c = self->first; c && (c != self->focus); c = c->next) {
                if (CAN_TILE_(c)) {
                    break;
                }
            }
        }
    } else if (dir == NAV_PREV) {
        PREV_CLIENT_(c, *self->focus->prev, CAN_TILE_(c));
        if (!c) {   /* loop */
            for (c = self->last; (c != *c->prev) && (c != self->focus); c = *c->prev) {
                if (CAN_TILE_(c)) {
                    break;
                }
            }
        }
    } else {
        NEXT_CLIENT_(c, self->first, CAN_TILE_(c));
    }
    if (c && c != self->focus) {
        swap_client(self->focus, c);
        update_client_geom(c);
        update_client_geom(self->focus);
    }
}

static
void move_tile(struct workspace_t *self, struct mouse_event_t *e) {
    (void)self;
    (void)e;
}

static
void resize_tile(struct workspace_t *self, struct mouse_event_t *e) {
    (void)self;
    (void)e;
}

static
void move_free(struct workspace_t *self, struct mouse_event_t *e) {
    struct client_t *c;

    (void)self;
    c = e->client;
    c->x += e->x2 - e->x1;
    c->y += e->y2 - e->y1;
    update_client_geom(c);
}

static
void resize_free(struct workspace_t *self, struct mouse_event_t *e) {
    struct client_t *c;

    (void)self;
    c = e->client;
    if ((e->x2 <= c->x) || (e->y2 <= c->y)) {
        return;
    }
    c->w = e->x2 - c->x;
    c->h = e->y2 - c->y;
    check_client_size(c);
    update_client_geom(c);
}

/** Handlers for each type of layouts
 */
static const struct layout_t layouts_[] = {
    [LAYOUT_TILE] = {
        .symbol     = SYMBOL_TILE_,
        .arrange    = &arrange_tile,
        .focus      = &focus_tile,
        .swap       = &swap_tile,
        .move       = &move_tile,
        .resize     = &resize_tile,
    },
    [LAYOUT_FREE] = {
        .symbol     = SYMBOL_FREE_,
        .arrange    = 0,
        .focus      = &focus_tile,
        .swap       = 0,
        .move       = &move_free,
        .resize     = &resize_free,
    },
};

/** Get layout handler of current workspace
 */
const struct layout_t *get_layout(struct workspace_t *self) {
    return &layouts_[self->layout];
}

void arrange_ws(struct workspace_t *self) {
    const struct layout_t *h;

    NIL_LOG("arrange %d", self - nil_.ws);
    h = &layouts_[self->layout];
    if (h->arrange) {
        (*h->arrange)(self);
    }
}

/** Hide all clients in the workspace
 */
void hide_ws(struct workspace_t *self)
{
    struct client_t *c;
    EACH_CLIENT_(self, c, hide_client);
}

void show_ws(struct workspace_t *self)
{
    struct client_t *c;
    EACH_CLIENT_(self, c, show_client);
}

/* vim: set ts=4 sw=4 expandtab: */
