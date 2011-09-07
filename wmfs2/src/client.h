/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "wmfs.h"

void client_configure(struct Client *c);
struct Client *client_gb_win(Window w);
void client_map(struct Client *c);
void client_unmap(struct Client *c);
void client_focus(struct Client *c);
void client_get_name(struct Client *c);
void client_close(struct Client *c);
struct Client *client_new(Window w, XWindowAttributes *wa);
void client_remove(struct Client *c);
void client_free(void);

#endif /* CLIENT_H */
