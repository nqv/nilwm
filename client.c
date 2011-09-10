/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <stdlib.h>
#include "nilwm.h"

/** Initialize a client after having window
 * @note client_t.win must be set previously.
 */
void init_client(struct client_t *self) {
    {   /* get window geometry */
        xcb_get_geometry_reply_t *geo;
        geo = xcb_get_geometry_reply(nil_.con,
            xcb_get_geometry(nil_.con, self->win), 0);
        if (geo) {
            self->x = geo->x;
            self->y = geo->y;
            self->w = geo->width;
            self->h = geo->height;
            free(geo);
        } else {
            self->x = 0;
            self->y = 0;
            self->w = nil_.scr->width_in_pixels;
            self->h = nil_.scr->height_in_pixels;
        }
    }
    NIL_LOG("client x=%d y=%d w=%d h=%d", self->x, self->y, self->w, self->h);
    self->tags = 0;
    self->flags = 0;
}

void config_client(struct client_t *self) {
    uint32_t vals[2];
    vals[0] = self->border_width;
    vals[1] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(nil_.con, self->win,
        XCB_CONFIG_WINDOW_BORDER_WIDTH | XCB_CONFIG_WINDOW_STACK_MODE,
        vals);
    vals[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE
        | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes(nil_.con, self->win, XCB_CW_EVENT_MASK, vals);
}

void move_resize_client(struct client_t *self) {
    uint32_t vals[4];
    vals[0] = self->x;
    vals[1] = self->y;
    vals[2] = self->w;
    vals[3] = self->h;
    xcb_configure_window(nil_.con, self->win,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
        | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        vals);
}

/** Add a client to the first position of client list
 */
void add_client(struct client_t *self, struct workspace_t *ws) {
    if (ws->client_first) {
        self->next = ws->client_first;
    } else {
        self->next = 0;
        ws->client_last = self;
    }
    ws->client_first = self;
}

struct client_t *find_client(xcb_window_t win) {
    struct client_t *c;
    unsigned int i;

    /* search in current workspace first :) */
    for (i = nil_.ws_idx; i < cfg_.num_workspaces; --i) {
        for (c = nil_.ws[i].client_first; c; c = c->next) {
            if (c->win == win) {
                return c;
            }
        }
    }
    for (i = nil_.ws_idx + 1; i < cfg_.num_workspaces; ++i) {
        for (c = nil_.ws[i].client_first; c; c = c->next) {
            if (c->win == win) {
                return c;
            }
        }
    }
    return 0;
}

struct client_t *remove_client(xcb_window_t win) {
    struct client_t *c, *p;
    unsigned int i;

    for (i = 0; i < cfg_.num_workspaces; ++i) {
        p = 0;
        c = nil_.ws[i].client_first;
        while (c) {
            if (c->win == win) {
                if (p) {
                    p->next = c->next;
                } else {    /* is the first window */
                    nil_.ws[i].client_first = c->next;
                }
                if (c->next == 0) { /* is also the last one */
                    nil_.ws[i].client_last = 0;
                }
                return c;
            }
            p = c;
            c = c->next;
        }
    }
    return 0;
}

void focus_client(struct client_t *self) {

}

void hide_client(struct client_t *self) {

}
/* vim: set ts=4 sw=4 expandtab: */
