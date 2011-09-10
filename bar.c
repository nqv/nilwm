/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <string.h>
#include "nilwm.h"

void draw_bar_text(struct bar_t *self, int x, int y, const char *str) {
    int len;

    len = strlen(str);
    xcb_image_text_8(nil_.con, len, self->win, self->gc, x, y, str);
}

/* vim: set ts=4 sw=4 expandtab: */
