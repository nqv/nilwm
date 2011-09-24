/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <string.h>
#include "nilwm.h"

struct bar_t bar_;

void text_bar(int x, int y, const char *str) {
    int len;

    len = strlen(str);
    xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, x, y, str);
}

void config_bar() {
    unsigned int i;
    int len, pad;
    xcb_rectangle_t rect;
    uint32_t vals[2];
    char text[3];

    /* workspace selection area */
    bar_.ws.x = 0;
    bar_.ws.w = bar_.h * cfg_.num_workspaces;

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
}

void update_bar_ws(unsigned int idx) {
    xcb_rectangle_t rect;
    uint32_t vals[2];
    char text[3];
    int len;

    /* colors */
    vals[0] = nil_.color.bar_fg;
    if (idx == nil_.ws_idx) {               /* focused workspace */
        vals[1] = nil_.color.focus;
    } else if (nil_.ws[idx].first) {        /* has a client */
        vals[1] = nil_.color.bar_hl;
    } else {
        vals[1] = nil_.color.bar_bg;
    }
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, &vals[1]);
    rect.width = bar_.ws.w / cfg_.num_workspaces;
    rect.height = bar_.h;
    rect.x = bar_.ws.x + rect.width * idx;
    rect.y = 0;
    xcb_poly_fill_rectangle(nil_.con, bar_.win, bar_.gc, 1, &rect);

    /* text */
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, vals);
    len = snprintf(text, sizeof(text), "%u", idx + 1);
    rect.x += (rect.width - cal_text_width(text, len)) / 2;
    rect.y += (rect.height + nil_.font.ascent + nil_.font.descent) / 2 - nil_.font.descent;
    xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, rect.x, rect.y, text);
}

/** Handle mouse click on bar
 */
int click_bar(int x) {
    /* click on workspace selection */
    if ((x >= bar_.ws.x) && (x <= (bar_.ws.x + bar_.ws.w))) {
        struct arg_t arg;
        /* which workspace */
        x += bar_.ws.x;
        arg.u = x / (bar_.ws.w / cfg_.num_workspaces);
        change_ws(&arg);
        return 1;
    }
    return 0;
}

/* vim: set ts=4 sw=4 expandtab: */
