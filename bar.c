/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <string.h>
#include "nilwm.h"

/* get padding for center alignment */
#define CENTER_H_(W, L)     (((W) - (L)) / 2)
#define CENTER_V_(H)        (((H) + nil_.font.ascent + nil_.font.descent) / 2 - nil_.font.descent)
#define STATUS_LEN_         128
#define STATUS_TEXT_        "NilWM"

struct bar_t bar_;
static char status_[STATUS_LEN_];

void text_bar(int x, int y, const char *str) {
    int len;

    len = strlen(str);
    xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, x, y, str);
}

static
void draw_bar_text(struct bar_box_t *box, const char *s, int len) {
    uint32_t vals[2];
    xcb_rectangle_t rect;

    vals[0] = nil_.color.bar_fg;
    vals[1] = nil_.color.bar_bg;
    if (box->w > 0) {           /* clear box before writing text */
        rect.x = box->x;
        rect.y = 0;
        rect.width = box->w;
        rect.height = bar_.h;
        xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, &vals[1]);
        xcb_poly_fill_rectangle(nil_.con, bar_.win, bar_.gc, 1, &rect);
    }
    /* new pos/size */
    rect.width = cal_text_width(s, len);
    if (NIL_HAS_FLAG(box->flags, BOX_FIXED)) {          /* size is not changed */
        /* text alignment */
        if (NIL_HAS_FLAG(box->flags, BOX_TEXT_CENTER)) {
            rect.x = box->x + CENTER_H_(box->w, rect.width);
        } else if (NIL_HAS_FLAG(box->flags, BOX_TEXT_RIGHT)) {
            rect.x = box->x + box->w - rect.width;
        } else {
            rect.x = box->x;
        }
    } else if (NIL_HAS_FLAG(box->flags, BOX_RIGHT)) {   /* float right */
        rect.x = box->x + box->w - rect.width;
        box->x = rect.x;
        box->w = rect.width;
    } else {                                            /* float left */
        rect.x = box->x;
        box->w = rect.width;
    }
    NIL_LOG("draw text %d in %d %u", rect.x, box->x, box->w);
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, vals);
    xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, rect.x,
        CENTER_V_(bar_.h), s);
}

/** Handle mouse click on workspace selection
 */
static
void click_ws(struct bar_box_t *self, int x) {
    struct arg_t arg;

    /* find which workspace */
    x += self->x;
    arg.u = x / (self->w / cfg_.num_workspaces);
    change_ws(&arg);
}

static
void click_sym(struct bar_box_t *NIL_UNUSED(self), int NIL_UNUSED(x)) {

}

void config_bar() {
    unsigned int i;
    int len, pad;
    xcb_rectangle_t rect;
    uint32_t vals[2];
    char text[3];
    struct bar_box_t *box;

    /* workspace selection area */
    box = &bar_.box[BAR_WS];
    box->x = 0;
    box->w = bar_.h * cfg_.num_workspaces;
    box->flags = BOX_FIXED;
    box->click = &click_ws;

    /* colors */
    vals[0] = nil_.color.bar_fg;
    vals[1] = nil_.color.bar_bg;
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, &vals[1]);

    rect.width = bar_.w;
    rect.height = bar_.h;
    rect.x = 0;
    rect.y = 0;
    xcb_poly_fill_rectangle(nil_.con, bar_.win, bar_.gc, 1, &rect);

    /* workspace numbers */
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, vals);
    rect.x = 0;
    rect.y = (rect.height + nil_.font.ascent + nil_.font.descent) / 2 - nil_.font.descent;
    rect.width = bar_.h;
    for (i = 1; i <= cfg_.num_workspaces; ++i) {
        len = snprintf(text, sizeof(text), "%u", i);
        pad = (rect.width - cal_text_width(text, len)) / 2; /* to align center */
        xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, rect.x + pad, rect.y, text);
        rect.x += rect.width;
    }
    /* layout symbol (next to ws) */
    box = &bar_.box[BAR_SYM];
    box->x = 0 + bar_.box[BAR_WS].w;
    box->w = 30;
    box->flags = BOX_FIXED | BOX_TEXT_CENTER;
    box->click = &click_sym;
    /* icon tray (right aligned) */
    box = &bar_.box[BAR_ICON];
    box->x = bar_.w;
    box->w = 0;                 /* no icon yet */
    box->flags = BOX_RIGHT;
    box->click = 0;
    /* status (right aligned) */
    box = &bar_.box[BAR_STATUS];
    box->x = bar_.w - bar_.box[BAR_ICON].w;
    box->w = 0;                 /* empty */
    box->flags = BOX_RIGHT;
    box->click = 0;
    /* task (remain) */
    box = &bar_.box[BAR_TASK];
    box->x = bar_.box[BAR_SYM].x + bar_.box[BAR_SYM].w;
    box->w = bar_.box[BAR_STATUS].x - bar_.box[BAR_TASK].x;
    box->flags = BOX_LEFT;
    box->click = 0;
}

/** Handle mouse click on bar
 */
int click_bar(int x) {
    struct bar_box_t *box;
    int i;

    for (i = NUM_BAR - 1; i >= 0; --i) {
        box = &bar_.box[i];
        if ((x >= box->x) && (x <= (box->x + box->w))) {
            if (box->click) {
                (*box->click)(box, x);
                return 1;
            }
            break;
        }
    }
    return 0;
}

void update_bar_ws(unsigned int idx) {
    xcb_rectangle_t rect;
    uint32_t vals[2];
    char text[3];
    int len;

    /* colors */
    vals[0] = nil_.color.bar_fg;
    if (idx == nil_.ws_idx) {               /* focused workspace */
        vals[1] = nil_.color.bar_sel;
    } else if (nil_.ws[idx].first) {        /* has a client */
        vals[1] = nil_.color.bar_occ;
    } else {
        vals[1] = nil_.color.bar_bg;
    }
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, &vals[1]);
    rect.width = bar_.box[BAR_WS].w / cfg_.num_workspaces;
    rect.height = bar_.h;
    rect.x = bar_.box[BAR_WS].x + rect.width * idx;
    rect.y = 0;
    xcb_poly_fill_rectangle(nil_.con, bar_.win, bar_.gc, 1, &rect);

    /* text */
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, vals);
    len = snprintf(text, sizeof(text), "%u", idx + 1);
    rect.x += CENTER_H_(rect.width, cal_text_width(text, len));
    rect.y += CENTER_V_(rect.height);
    xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, rect.x, rect.y, text);
}

void update_bar_sym() {
    const char *sym;

    sym = (get_layout(nil_.ws_idx))->symbol;
    if (!sym) {
        NIL_ERR("no layout symbol %d", nil_.ws_idx);
        return;
    }
    draw_bar_text(&bar_.box[BAR_SYM], sym, strlen(sym));
}

void update_bar_status() {
    int len;
    len = get_text_prop(nil_.scr->root, XCB_ATOM_WM_NAME, status_, sizeof(status_));
    if (len <= 0) {
        len = strlen(STATUS_TEXT_);
        memcpy(status_, STATUS_TEXT_, len);
        status_[len] = '\0';
    }
    draw_bar_text(&bar_.box[BAR_STATUS], status_, len);
}

/* vim: set ts=4 sw=4 expandtab: */
