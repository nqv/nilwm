/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#ifndef NILWM_CONFIG_H_
#define NILWM_CONFIG_H_

#define MODKEY	XCB_MOD_MASK_4

const char *CMD_TERM[] = { "xterm", 0 };

const struct shortcut_t SHORTCUTS[] = {
    /* modifier                 key             function        argument */
    { MODKEY,                   0,              spawn,          { 0, CMD_TERM } },
};

#endif /* NILWM_CONFIG_H_ */
