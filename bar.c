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
    int len;
    xcb_rectangle_t rect;
    uint32_t vals[1];
    char text[3];

    /* workspace selection area */
    bar_.ws.x = 0;
    bar_.ws.w = bar_.h * cfg_.num_workspaces;

    /* background color */
    vals[0] = nil_.color.bar_bg;
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, vals);

    rect.width = bar_.w;
    rect.height = bar_.h;
    rect.x = 0;
    rect.y = 0;
    xcb_poly_fill_rectangle(nil_.con, bar_.win, bar_.gc, 1, &rect);

    /* workspace numbers */
    vals[0] = nil_.color.bar_fg;
    xcb_change_gc(nil_.con, bar_.gc, XCB_GC_FOREGROUND, vals);

    rect.x = 2;                     /* padding */
    rect.y = rect.height - 2;
    for (i = 1; i <= cfg_.num_workspaces; ++i) {
        len = snprintf(text, sizeof(text), "%u", i);
        xcb_image_text_8(nil_.con, len, bar_.win, bar_.gc, rect.x, rect.y, text);
        rect.x += bar_.h;
    }
}

void click_bar(int x) {

}

/* vim: set ts=4 sw=4 expandtab: */
