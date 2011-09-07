/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
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
};
