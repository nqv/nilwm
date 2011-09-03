/*
 * Nilwm - Lightweight X window manager.
 * See LICENSE file for copyright and license details.
 * vim:ts=4:sw=4:expandtab
 */

#include "nilwm.h"

/**
 * client_t.win must be set previously.
 */
void init_client(struct client_t *self) {
    self->x = 0;
    self->y = 0;
    self->w = 0;
    self->h = 0;

    self->tags = 0;
    self->flags = 0;
}

/**
 * Add a client to the client list
 */
void add_client(struct client_t *self) {
    if (nil_.client_list) {
        self->next = nil_.client_list;
    } else {
        self->next = 0;
    }
    nil_.client_list = self;
}

void focus_client(struct client_t *self) {

}

void hide_client(struct client_t *self) {

}

