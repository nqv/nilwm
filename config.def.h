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
#define BAR_BG_COLOR        "blue"
#define BAR_FG_COLOR        "white"
#define BAR_HL_COLOR        "yellow"

#define FONT_NAME           "-*-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-*"

static const char *CMD_TERM[] = { "xterm", 0 };

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
    { MOD_KEY,                      XK_1,           change_ws,          {.u =  0} },
    { MOD_KEY,                      XK_2,           change_ws,          {.u =  1} },
    { MOD_KEY,                      XK_3,           change_ws,          {.u =  2} },
    { MOD_KEY,                      XK_4,           change_ws,          {.u =  3} },
    { MOD_KEY,                      XK_5,           change_ws,          {.u =  4} },
    { MOD_KEY,                      XK_6,           change_ws,          {.u =  5} },
    { MOD_KEY,                      XK_7,           change_ws,          {.u =  6} },
    { MOD_KEY,                      XK_8,           change_ws,          {.u =  7} },
    { MOD_KEY,                      XK_9,           change_ws,          {.u =  8} },
};

#endif /* NILWM_CONFIG_H_ */
/* vim: set ts=4 sw=4 expandtab: */
