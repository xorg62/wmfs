/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "wmfs.h"

void client_configure(struct client *c);
struct client *client_gb_win(Window w);
struct client *client_next_with_pos(struct client *bc, Position p);
void client_focus(struct client *c);
void client_get_name(struct client *c);
void client_close(struct client *c);
struct client *client_new(Window w, XWindowAttributes *wa);
void client_moveresize(struct client *c, struct geo g);
void client_maximize(struct client *c);
void client_remove(struct client *c);
void client_free(void);

#endif /* CLIENT_H */
