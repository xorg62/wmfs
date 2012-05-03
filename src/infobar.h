/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include "wmfs.h"
#include "util.h"
#include "tag.h"

enum { ElemTag = 0, ElemStatus, ElemSystray, ElemLauncher, ElemCustom, ElemLast };

struct infobar *infobar_new(struct screen *s, char *name, struct theme *theme, enum barpos pos, const char *elem);
void infobar_elem_update(struct infobar *i, int type);
void infobar_refresh(struct infobar *i);
void infobar_remove(struct infobar *i);
void infobar_free(struct screen *s);
void infobar_elem_reinit(struct infobar *i);

/* Basic placement of elements */
static inline void
infobar_elem_placement(struct element *e)
{
     struct element *p = TAILQ_PREV(e, esub, next);

     e->geo.y = 0;
     e->geo.h = e->infobar->geo.h;

     if(e->align == Left)
          e->geo.x = (p ? p->geo.x + p->geo.w : 0);
     else
          e->geo.x = ((p = TAILQ_NEXT(e, next))
                       ? p->geo.x - e->geo.w
                       : e->infobar->geo.w - e->geo.w);
}

/* Bars placement management and usable space management */
static inline bool
infobar_placement(struct infobar *i, enum barpos p)
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

     return true;
}

static inline void
infobar_elem_screen_update(struct screen *s, int type)
{
     struct infobar *i;

     SLIST_FOREACH(i, &s->infobars, next)
          infobar_elem_update(i, type);

}

static inline struct infobar*
infobar_gb_name(const char *name)
{
     struct screen *s;
     struct infobar *i;

     SLIST_FOREACH(s, &W->h.screen, next)
     {
          SLIST_FOREACH(i, &s->infobars, next)
               if(!strcmp(i->name, name))
                    return i;
     }

     return SLIST_FIRST(&s->infobars);
}

void uicb_infobar_toggle_hide(Uicb iname);

#endif /* INFOBAR_H */
