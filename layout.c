/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include "nilwm.h"

#define RSZ_CLIENT_(C, X, Y, W, H)  C->x = X; C->y = Y; \
    C->w = (W) - 2 * C->border_width; C->h = (H) - 2 * C->border_width

struct layout_handler_t {
    void (*arrange)(struct workspace_t *);
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
        if (NIL_HAS_FLAG(m->flags, CLIENT_MAPPED) && !NIL_HAS_FLAG(m->flags, CLIENT_FLOAT)) {
            break;
        }
    }
    if (!m) {
        return;
    }
    /* next client with is not float */
    for (c = m->next; c; c = c->next) {
        if (NIL_HAS_FLAG(m->flags, CLIENT_MAPPED) && !NIL_HAS_FLAG(c->flags, CLIENT_FLOAT)) {
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
        if (NIL_HAS_FLAG(m->flags, CLIENT_MAPPED) && !NIL_HAS_FLAG(m->flags, CLIENT_FLOAT)) {
            ++n;
        }
    }
    h = nil_.h / n;
    do {
        if (!NIL_HAS_FLAG(c->flags, CLIENT_MAPPED) || NIL_HAS_FLAG(c->flags, CLIENT_FLOAT)) {
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

static
void arrange_free(struct workspace_t *self) {

}

/** Handlers for each type of layouts
 */
static const struct layout_handler_t layouts_[] = {
    [LAYOUT_TILE]  = { .arrange = &arrange_tile },
    [LAYOUT_FREE]   = { .arrange = &arrange_free },
};

void arrange(struct workspace_t *self) {
    NIL_LOG("arrange workspace %d", self - nil_.ws);
    if (layouts_[self->layout].arrange) {
        (*layouts_[self->layout].arrange)(self);
    }
}
/* vim: set ts=4 sw=4 expandtab: */
