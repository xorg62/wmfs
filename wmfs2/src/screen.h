/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "wmfs.h"

static inline Scr33n*
screen_gb_id(int id)
{
     Scr33n *s;

     SLIST_FOREACH(s, &W->h.screen, next)
          if(s->id == id)
               return s;

     return SLIST_FIRST(&W->h.screen);
}

void screen_init(void);
Scr33n* screen_update_sel(void);
void screen_free(void);

#endif /* SCREEN_H */
