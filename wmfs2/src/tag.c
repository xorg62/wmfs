/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "tag.h"
#include "util.h"
#include "infobar.h"

Tag*
tag_new(Scr33n *s, char *name)
{
     Tag *t;

     t = xcalloc(1, sizeof(Tag));

     t->screen = s;
     t->name   = name;
     t->flags  = 0;
     t->sel    = NULL;

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(Scr33n *s, Tag *t)
{
     Infobar *i;

     s->seltag = t;

     SLIST_FOREACH(i, &s->infobars, next)
     {
          i->elemupdate |= FLAGINT(ElemTag);
          infobar_elem_update(i);
     }
}

void
uicb_tag_set(Uicb cmd)
{
     int i = 0, n = ATOI(cmd);
     Tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(++i == n)
          {
               tag_screen(W->screen, t);
               return;
          }
}

void
uicb_tag_next(Uicb cmd)
{
     (void)cmd;
     Tag *t;

     if((t = TAILQ_NEXT(W->screen->seltag, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_FIRST(&W->screen->tags));
}

void
uicb_tag_prev(Uicb cmd)
{
     (void)cmd;
     Tag *t;

     if((t = TAILQ_PREV(W->screen->seltag, tsub, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_LAST(&W->screen->tags, tsub));
}


void
tag_free(Scr33n *s)
{
     Tag *t;

     TAILQ_FOREACH(t, &s->tags, next)
     {
          TAILQ_REMOVE(&s->tags, t, next);
          free(t);
     }
}
