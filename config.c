/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#include <X11/keysym.h>
#include "nilwm.h"
#include "config.h"

/* TODO: load config from rc file */
const struct config_t cfg_ = {
    .mod_key = MOD_KEY,
    .border_width = BORDER_WIDTH,
    .num_workspaces = NUM_WORKSPACES,

    .keys = (struct key_t *)KEYS,
    .keys_len = NIL_LEN(KEYS),

    .master_size = MASTER_SIZE,
    .font_name = FONT_NAME,

    .border_color = BORDER_COLOR,
    .focus_color = FOCUS_COLOR,
    .bar_bg_color = BAR_BG_COLOR,
    .bar_fg_color = BAR_FG_COLOR,
    .bar_hl_color = BAR_HL_COLOR,
};
/* vim: set ts=4 sw=4 expandtab: */
