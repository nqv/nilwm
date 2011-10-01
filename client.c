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
    xcb_size_hints_t sz;
    xcb_get_property_cookie_t cookie;

    if (self->w == 0 || self->h == 0) {         /* get window geometry */
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
        NIL_LOG("client x=%d y=%d w=%d h=%d", self->x, self->y, self->w, self->h);
    }
    /* get size hints */
    cookie = xcb_icccm_get_wm_normal_hints_unchecked(nil_.con, self->win);
    if (!xcb_icccm_get_wm_normal_hints_reply(nil_.con, cookie, &sz, 0)) {
        NIL_LOG("no normal hints %d", cookie.sequence);
        self->min_w = self->min_h = self->max_w = self->max_h = 0;
        self->flags = 0;
        return;
    }
    if (NIL_HAS_FLAG(sz.flags, XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)) {
        self->min_w = sz.min_width;
        self->min_h = sz.min_height;
    } else {
        self->min_w = self->min_h = 0;
    }
    if (NIL_HAS_FLAG(sz.flags, XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)) {
        self->max_w = sz.max_width;
        self->max_h = sz.max_height;
    } else {
        self->max_w = self->max_h = 0;
    }
    self->flags = 0;
    if (self->min_w && self->min_h && self->max_w && self->max_h
        && (self->min_w == self->max_w) && (self->min_h == self->max_h)) {
        NIL_SET_FLAG(self->flags, CLIENT_FLOAT | CLIENT_FIXED); /* force float */
    }
    NIL_LOG("hints min=%u,%u max=%u,%u flag=%u", self->min_w, self->min_h,
        self->max_w, self->max_h, self->flags);
}

void config_client(struct client_t *self) {
    uint32_t vals[2];
    vals[0] = self->border_width;
    vals[1] = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(nil_.con, self->win, XCB_CONFIG_WINDOW_BORDER_WIDTH
        | XCB_CONFIG_WINDOW_STACK_MODE, vals);
    vals[0] = nil_.color.border;
    xcb_change_window_attributes(nil_.con, self->win, XCB_CW_BORDER_PIXEL, vals);
    vals[0] = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE
        | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes(nil_.con, self->win, XCB_CW_EVENT_MASK, vals);
}

void check_client_size(struct client_t *self) {
    if (self->min_w && (self->w < self->min_w)) {
        self->w = self->min_w;
    } else if (self->max_w && (self->w > self->max_w)) {
        self->w = self->max_w;
    }
    if (self->min_h && (self->h < self->min_h)) {
        self->h = self->min_h;
    } else if (self->max_h && (self->h > self->max_h)) {
        self->h = self->max_h;
    }
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
void attach_client(struct client_t *self, struct workspace_t *ws) {
    if (ws->first) {
        ws->first->prev = self;
        self->next = ws->first;
    } else {
        self->next = 0;
        ws->last = self;
    }
    self->prev = 0;
    ws->first = self;
}

/** Client should be in the workspace
 */
void detach_client(struct client_t *self, struct workspace_t *ws) {
    if (self->prev) {
        self->prev->next = self->next;
    } else {    /* is the first window */
        ws->first = self->next;
    }
    if (self->next) {
        self->next->prev = self->prev;
    } else {    /* is the last one */
        ws->last = self->prev;
    }
}

struct client_t *find_client(xcb_window_t win, struct workspace_t **ws) {
    struct client_t *c;
    unsigned int i;

    /* search in current workspace first :) */
    for (i = nil_.ws_idx; i < cfg_.num_workspaces; --i) {
        for (c = nil_.ws[i].first; c; c = c->next) {
            if (c->win == win) {
                if (ws) {
                    *ws = &nil_.ws[i];
                }
                return c;
            }
        }
    }
    for (i = nil_.ws_idx + 1; i < cfg_.num_workspaces; ++i) {
        for (c = nil_.ws[i].first; c; c = c->next) {
            if (c->win == win) {
                if (ws) {
                    *ws = &nil_.ws[i];
                }
                return c;
            }
        }
    }
    return 0;
}

/** Find window and remove its client from found workspace
 */
struct client_t *remove_client(xcb_window_t win, struct workspace_t **ws) {
    struct client_t *c;
    unsigned int i;

    for (i = 0; i < cfg_.num_workspaces; ++i) {
        for (c = nil_.ws[i].first; c; c = c->next) {
            if (c->win == win) {
                detach_client(c, &nil_.ws[i]);
                if (ws) {   /* return the workspace affected */
                    *ws = &nil_.ws[i];
                }
                return c;
            }
        }
    }
    return 0;
}

/** Swap window, keep other properties
 */
void swap_client(struct client_t *self, struct client_t *c) {
    xcb_window_t win;

    win = self->win;
    self->win = c->win;
    c->win = win;
}

/** Change border color
 */
void focus_client(struct client_t *self) {
    uint32_t vals[1];

    vals[0] = nil_.color.focus;
    xcb_change_window_attributes(nil_.con, self->win, XCB_CW_BORDER_PIXEL, vals);
    NIL_SET_FLAG(self->flags, CLIENT_FOCUS);
}

/** Change border color
 */
void blur_client(struct client_t *self) {
    uint32_t vals[1];

    vals[0] = nil_.color.border;
    xcb_change_window_attributes(nil_.con, self->win, XCB_CW_BORDER_PIXEL, vals);
    NIL_CLEAR_FLAG(self->flags, CLIENT_FOCUS);
}

void raise_client(struct client_t *self) {
    const uint32_t vals[] = { XCB_STACK_MODE_ABOVE };
    xcb_configure_window(nil_.con, self->win, XCB_CONFIG_WINDOW_STACK_MODE, vals);
}

void hide_client(struct client_t *self) {
    xcb_unmap_window(nil_.con, self->win);
}

void show_client(struct client_t *self) {
    xcb_map_window(nil_.con, self->win);
}

/* vim: set ts=4 sw=4 expandtab: */
