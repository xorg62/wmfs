/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "tag.h"
#include "util.h"
#include "infobar.h"
#include "client.h"
#include "frame.h"

struct tag*
tag_new(struct screen *s, char *name)
{
     struct tag *t;

     t = xcalloc(1, sizeof(struct tag));

     t->screen = s;
     t->name   = xstrdup(name);
     t->flags  = 0;
     t->sel    = NULL;

     SLIST_INIT(&t->clients);
     SLIST_INIT(&t->frames);

     /* only one frame for now, *tmp* */
     frame_new(t); /* t->frame */

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(struct screen *s, struct tag *t)
{
     struct frame *f;
     struct client *c;

     /* Hide previous tag's frame */
     SLIST_FOREACH(f, &s->seltag->frames, next)
          frame_unmap(f);

     /* Unhide selected tag's clients */
     SLIST_FOREACH(f, &t->frames, next)
          frame_update(f);

     s->seltag = t;

     client_focus(t->sel);

     infobar_elem_screen_update(s, ElemTag);
}

void
tag_client(struct tag *t, struct client *c)
{
     /* Remove client from its previous tag */
     if(c->tag)
     {
          if(c->tag == t)
               return;

          SLIST_REMOVE(&c->tag->clients, c, client, tnext);

          if(c->tag->sel == c)
               c->tag->sel = NULL;
     }

     /* Case of client remove */
     if(!t)
          return;

     c->tag = t;

     /* Insert in new tag list */
     SLIST_INSERT_HEAD(&t->clients, c, tnext);
}

void
uicb_tag_set(Uicb cmd)
{
     int i = 0, n = ATOI(cmd);
     struct tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(++i == n)
          {
               tag_screen(W->screen, t);
               return;
          }
}

void
uicb_tag_set_with_name(Uicb cmd)
{
     struct tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(!strcmp(cmd, t->name))
          {
               tag_screen(W->screen, t);
               return;
          }
}

void
uicb_tag_next(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if((t = TAILQ_NEXT(W->screen->seltag, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_FIRST(&W->screen->tags));
}

void
uicb_tag_prev(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if((t = TAILQ_PREV(W->screen->seltag, tsub, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_LAST(&W->screen->tags, tsub));
}

static void
tag_remove(struct tag *t)
{
     struct frame *f;

     free(t->name);

     frame_free(t);

     free(t);
}

void
tag_free(struct screen *s)
{
     struct tag *t;

     TAILQ_FOREACH(t, &s->tags, next)
     {
          TAILQ_REMOVE(&s->tags, t, next);
          tag_remove(t);
     }
}
