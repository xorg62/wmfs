/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef MOUSE_H
#define MOUSE_H

#include "client.h"

void uicb_mouse_resize(Uicb);
void uicb_mouse_move(Uicb);
void uicb_mouse_tab(Uicb);
void uicb_mouse_integrate(Uicb);

static inline bool
mouse_check_client(struct client *c)
{
     Window w;
     int d;

     XQueryPointer(W->dpy, W->root, &w, &w, &d, &d, &d, &d, (uint *)&d);

     if(c == client_gb_win(w) || c == client_gb_titlebar(w) || c == client_gb_frame(w))
          return true;

     return false;
}

#endif /* MOUSE_H */
