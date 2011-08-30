/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include "wmfs.h"
#include "util.h"
#include "draw.h"

#define INFOBAR_DEF_W (14)

enum { ElemTag = 0, ElemLayout, ElemSelbar, ElemStatus, ElemCustom, ElemLast };

Infobar *infobar_new(Scr33n *s, const char *elem);
void infobar_elem_update(Infobar *i);
void infobar_refresh(Infobar *i);
void infobar_remove(Infobar *i);
void infobar_free(Scr33n *s);

/* Basic placement of elements */
static inline void
infobar_elem_placement(Element *e)
{
     Element *p = TAILQ_PREV(e, esub, next);

     e->geo.y = e->geo.w = 0;
     e->geo.h = e->infobar->geo.h;
     e->geo.x = (p ? p->geo.x + p->geo.w + PAD : 0);
}

static inline void
infobar_placement(Infobar *i)
{
     Infobar *h = SLIST_FIRST(&i->screen->infobars);

     i->geo = i->screen->geo;
     i->geo.h =  W->conf.theme.bars_width;
     i->geo.y += (h ? h->geo.y + h->geo.h : i->screen->geo.y);
}

static inline void
infobar_elem_screen_update(Scr33n *s, int addf)
{
     Infobar *i;

     s->elemupdate |= FLAGINT(addf);

     SLIST_FOREACH(i, &s->infobars, next)
          infobar_elem_update(i);

     s->elemupdate &= ~FLAGINT(addf);
}

#endif /* INFOBAR_H */
