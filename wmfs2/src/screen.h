/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "wmfs.h"

static inline struct screen*
screen_gb_id(int id)
{
     struct screen *s;

     SLIST_FOREACH(s, &W->h.screen, next)
          if(s->id == id)
               return s;

     return SLIST_FIRST(&W->h.screen);
}

void screen_init(void);
struct screen* screen_update_sel(void);
void screen_free(void);

#endif /* SCREEN_H */
