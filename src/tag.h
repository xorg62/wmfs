/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef TAG_H
#define TAG_H

#include "wmfs.h"


static inline struct tag*
tag_gb_id(struct screen *s, int id)
{
     struct tag *t;

     TAILQ_FOREACH(t, &s->tags, next)
          if(t->id == id)
               return t;

     return TAILQ_FIRST(&s->tags);
}

struct tag *tag_new(struct screen *s, char *name);
void tag_screen(struct screen *s, struct tag *t);
void tag_client(struct tag *t, struct client *c);
void tag_free(struct screen *s);
void uicb_tag_set(Uicb cmd);
void uicb_tag_set_with_name(Uicb cmd);
void uicb_tag_next(Uicb cmd);
void uicb_tag_prev(Uicb cmd);

#endif /* TAG_H */
