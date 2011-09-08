/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef FRAME_H
#define FRAME_H

#include "wmfs.h"

#define FRAME_EMPTY(f) SLIST_EMPTY(&f->clients)

/*
 * Update each frame's geo of a screen after
 * usable geo update (infobar_placement())
 */
static inline void
frame_update_geo(struct screen *s)
{
     struct tag *t;
     struct frame *f;

     TAILQ_FOREACH(t, &s->tags, next)
     {
          SLIST_FOREACH(f, &t->frames, next)
               f->geo = s->ugeo;
     }
}

struct frame *frame_new(struct tag *t);
void frame_free(struct tag *t);
void frame_update(struct frame *f);
void frame_map(struct frame *f);
void frame_unmap(struct frame *f);

#endif /* FRAME_H */

