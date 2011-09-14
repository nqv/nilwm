/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 */

#ifndef NILWM_CONFIG_H_
#define NILWM_CONFIG_H_

/* See xcb/xproto.h */
#define MOD_KEY	            XCB_MOD_MASK_4
#define MOD_SHIFT           XCB_MOD_MASK_SHIFT
#define MOD_CTRL            XCB_MOD_MASK_CONTROL

#define BORDER_WIDTH        1
#define NUM_WORKSPACES      9
#define MASTER_FACTOR       0.55

#define BORDER_COLOR        "blue"
#define FOCUS_COLOR         "red"

#define FONT_NAME           "-*-fixed-*-*-*-*-14-*-*-*-*-*-*-*"

static const char *CMD_TERM[] = { "xterm", 0 };

/* Keysym X11/keysymdefs.h */
static const struct key_t KEYS[] = {
    /* modifier                     key             function        argument */
    { MOD_KEY|MOD_CTRL,             XK_Return,      spawn,          { 0, CMD_TERM } },
    { MOD_KEY,                      XK_j,           focus,          { +1, 0 } },
    { MOD_KEY,                      XK_k,           focus,          { -1, 0 } },
};

#endif /* NILWM_CONFIG_H_ */
/* vim: set ts=4 sw=4 expandtab: */
