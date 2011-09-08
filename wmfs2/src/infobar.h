/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include "wmfs.h"
#include "util.h"
#include "draw.h"
#include "frame.h"

enum { ElemTag = 0, ElemLayout, ElemSelbar, ElemStatus, ElemCustom, ElemLast };

struct infobar *infobar_new(struct screen *s, struct theme *theme, Barpos pos, const char *elem);
void infobar_elem_update(struct infobar *i);
void infobar_refresh(struct infobar *i);
void infobar_remove(struct infobar *i);
void infobar_free(struct screen *s);

/* Basic placement of elements */
static inline void
infobar_elem_placement(struct element *e)
{
     struct element *p = TAILQ_PREV(e, esub, next);

     e->geo.y = e->geo.w = 0;
     e->geo.h = e->infobar->geo.h;
     e->geo.x = (p ? p->geo.x + p->geo.w + PAD : 0);
}

/* Bars placement management and usable space management */
static inline bool
infobar_placement(struct infobar *i, Barpos p)
{
     i->pos = p;
     i->geo = i->screen->ugeo;
     i->geo.h = i->theme->bars_width;

     switch(p)
     {
          case BarTop:
               i->screen->ugeo.y += i->geo.h;
               i->screen->ugeo.h -= i->geo.h;
               break;
          case BarBottom:
               i->geo.y = (i->screen->ugeo.y + i->screen->ugeo.h) - i->geo.h;
               i->screen->ugeo.h -= i->geo.h;
               break;
          default:
          case BarHide:
               return false;
     }

     frame_update_geo(i->screen);

     return true;
}

static inline void
infobar_elem_screen_update(struct screen *s, int addf)
{
     struct infobar *i;

     s->elemupdate |= FLAGINT(addf);

     SLIST_FOREACH(i, &s->infobars, next)
          infobar_elem_update(i);

     s->elemupdate &= ~FLAGINT(ElemTag);
}

#endif /* INFOBAR_H */
