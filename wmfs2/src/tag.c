/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "tag.h"
#include "util.h"

Tag*
tag_new(Scr33n *s, char *name)
{
     Tag *t;

     t = xcalloc(1, sizeof(Tag));

     t->screen = s;
     t->name   = name;
     t->flags  = 0;
     t->sel    = NULL;

     SLIST_INSERT_HEAD(&s->tags, t, next);

     return t;
}

void
tag_screen(Scr33n *s, Tag *t)
{
     s->seltag = t;

}


void
tag_free(Scr33n *s)
{
     FREE_LIST(Tag, s->tags);
}
