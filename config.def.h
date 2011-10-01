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
#define MASTER_SIZE         55      /* % */

#define BORDER_COLOR        "blue"
#define FOCUS_COLOR         "red"
#define BAR_BG_COLOR        "#111111"
#define BAR_FG_COLOR        "#AAAAAA"
#define BAR_SEL_COLOR       "#003366"       /* selecting */
#define BAR_OCC_COLOR       "#333333"       /* occupied */
#define BAR_URG_COLOR       "#663300"       /* urgent */

#define FONT_NAME           "-*-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-*"

static const char *CMD_TERM[] = { "xterm", 0 };

#define KEY_WS_(KEY, NUM)   \
    { MOD_KEY,                      KEY,            change_ws,          {.u = NUM} }, \
    { MOD_KEY|MOD_SHIFT,            KEY,            push,               {.u = NUM} },

/* Keysym X11/keysymdefs.h */
static const struct key_t KEYS[] = {
    /* modifier                     key             function            argument */
    { MOD_KEY|MOD_CTRL,             XK_Return,      spawn,              {.v = CMD_TERM} },
    { MOD_KEY,                      XK_Return,      focus,              {.i =  0} },
    { MOD_KEY,                      XK_j,           focus,              {.i = +1} },
    { MOD_KEY,                      XK_k,           focus,              {.i = -1} },
    { MOD_KEY,                      XK_l,           set_msize,          {.i = +5} },
    { MOD_KEY,                      XK_h,           set_msize,          {.i = -5} },
    { MOD_KEY|MOD_SHIFT,            XK_Return,      swap,               {.i =  0} },
    { MOD_KEY|MOD_SHIFT,            XK_j,           swap,               {.i = +1} },
    { MOD_KEY|MOD_SHIFT,            XK_k,           swap,               {.i = -1} },
    { MOD_KEY|MOD_SHIFT,            XK_c,           kill_focused,       {.i =  0} },
    { MOD_KEY|MOD_SHIFT,            XK_space,       toggle_floating,    {.i =  0} },
    KEY_WS_(                        XK_1,                               0)
    KEY_WS_(                        XK_2,                               1)
    KEY_WS_(                        XK_3,                               2)
    KEY_WS_(                        XK_4,                               3)
    KEY_WS_(                        XK_5,                               4)
    KEY_WS_(                        XK_6,                               5)
    KEY_WS_(                        XK_7,                               6)
    KEY_WS_(                        XK_8,                               7)
    KEY_WS_(                        XK_9,                               8)
    { MOD_KEY|MOD_SHIFT|MOD_CTRL,   XK_q,           quit,               {.i =  0} },
};

#endif /* NILWM_CONFIG_H_ */
/* vim: set ts=4 sw=4 expandtab: */
