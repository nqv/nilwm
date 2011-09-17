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
#define BAR_BG_COLOR        "blue"
#define BAR_FG_COLOR        "white"
#define BAR_HL_COLOR        "yellow"

#define FONT_NAME           "-*-fixed-medium-*-*-*-14-*-*-*-*-*-*-*"

static const char *CMD_TERM[] = { "xterm", 0 };

/* Keysym X11/keysymdefs.h */
static const struct key_t KEYS[] = {
    /* modifier                     key             function        argument */
    { MOD_KEY|MOD_CTRL,             XK_Return,      spawn,          { .v = CMD_TERM } },
    { MOD_KEY,                      XK_Return,      focus,          { .i = 0 } },
    { MOD_KEY,                      XK_j,           focus,          { .i = +1 } },
    { MOD_KEY,                      XK_k,           focus,          { .i = -1 } },
    { MOD_KEY|MOD_SHIFT,            XK_Return,      swap,           { .i = 0 } },
    { MOD_KEY|MOD_SHIFT,            XK_j,           swap,           { .i = +1 } },
    { MOD_KEY|MOD_SHIFT,            XK_k,           swap,           { .i = -1 } },
};

#endif /* NILWM_CONFIG_H_ */
/* vim: set ts=4 sw=4 expandtab: */
