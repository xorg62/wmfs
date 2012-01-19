/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "wmfs.h"
#include "util.h"

static inline struct screen*
screen_gb_id(int id)
{
     struct screen *s;

     SLIST_FOREACH(s, &W->h.screen, next)
          if(s->id == id)
               return s;

     return SLIST_FIRST(&W->h.screen);
}

static inline struct screen*
screen_gb_mouse(void)
{
     struct screen *s;
     Window w;
     int d, x, y;

     XQueryPointer(W->dpy, W->root, &w, &w, &x, &y, &d, &d, (unsigned int *)&d);

     SLIST_FOREACH(s, &W->h.screen, next)
          if(INAREA(x, y, s->geo))
               break;

     if(!s)
          s = SLIST_FIRST(&W->h.screen);

     return s;
}

void screen_init(void);
struct screen* screen_update_sel(void);
void screen_free(void);
void uicb_screen_next(Uicb cmd);
void uicb_screen_prev(Uicb cmd);
void uicb_screen_move_client_next(Uicb cmd);
void uicb_screen_move_client_prev(Uicb cmd);


#endif /* SCREEN_H */
