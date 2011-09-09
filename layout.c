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

static
void arrange_tile(struct workspace_t *self) {
    struct client_t *c;
    int16_t x, y;
    uint16_t w, h;
    unsigned int n;

    /* master */
    c = self->client_first;
    if (!c) {
        return;
    }
    if (!c->next) {     /* 1 window */
        RSZ_CLIENT_(c, 0, 0, nil_.scr->width_in_pixels, nil_.scr->height_in_pixels);
        NIL_LOG("arrange master %d,%d %ux%u", c->x, c->y, c->w, c->h);
        return;
    }
    w = cfg_.mfact * nil_.scr->width_in_pixels;
    RSZ_CLIENT_(c, 0, 0, w, nil_.scr->height_in_pixels);
    /* next position */
    x = (int)w;
    y = 0;
    w = nil_.scr->width_in_pixels - w;
    /* get number of clients */
    n = 1;
    for (c = c->next->next; c; c = c->next) {
        ++n;
    }
    h = nil_.scr->height_in_pixels / n;
    NIL_LOG("arrange client (%d) %u,%u", n, w, h);
    for (c = self->client_first->next; c; c = c->next) {
        if (c == self->client_last) {
            RSZ_CLIENT_(c, x, y, w, nil_.scr->height_in_pixels - y);
        } else {
            RSZ_CLIENT_(c, x, y, w, h);
            y = c->y + c->h + 2 * c->border_width;
        }
    }
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
    (*layouts_[self->layout].arrange)(self);
}
/* vim: set ts=4 sw=4 expandtab: */
