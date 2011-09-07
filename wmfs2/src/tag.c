/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "tag.h"
#include "util.h"
#include "infobar.h"
#include "client.h"
#include "frame.h"

struct Tag*
tag_new(struct Scr33n *s, char *name)
{
     struct Tag *t;

     t = xcalloc(1, sizeof(struct Tag));

     t->screen = s;
     t->name   = xstrdup(name);
     t->flags  = 0;
     t->sel    = NULL;

     SLIST_INIT(&t->clients);
     SLIST_INIT(&t->frames);

     /*
      * tmp
      */
     t->frame = frame_new(t);

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(struct Scr33n *s, struct Tag *t)
{
     struct Frame *f;
     struct Client *c;

     /* Hide previous tag's clients */
     if(s->seltag)
          SLIST_FOREACH(c, &s->seltag->clients, tnext)
               client_unmap(c);

     s->seltag = t;

     /* Unhide selected tag's clients */
     SLIST_FOREACH(c, &t->clients, tnext)
          client_map(c);

     client_focus(t->sel);

     infobar_elem_screen_update(s, ElemTag);

     SLIST_FOREACH(f, &t->frames, next)
          frame_update(f);
}

void
tag_client(struct Tag *t, struct Client *c)
{
     /* Remove client from its previous tag */
     if(c->tag)
     {
          SLIST_REMOVE(&c->tag->clients, c, Client, tnext);

          if(c->tag->sel == c)
               c->tag->sel = NULL;
     }

     /* Case of client remove, t = NULL */
     if(!(c->tag = t))
          return;

     /* Insert in new tag list */
     SLIST_INSERT_HEAD(&t->clients, c, tnext);
}


void
uicb_tag_set(Uicb cmd)
{
     int i = 0, n = ATOI(cmd);
     struct Tag *t;

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
     struct Tag *t;

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
     struct Tag *t;

     if((t = TAILQ_NEXT(W->screen->seltag, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_FIRST(&W->screen->tags));
}

void
uicb_tag_prev(Uicb cmd)
{
     (void)cmd;
     struct Tag *t;

     if((t = TAILQ_PREV(W->screen->seltag, tsub, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_LAST(&W->screen->tags, tsub));
}

static void
tag_remove(struct Tag *t)
{
     struct Frame *f;

     free(t->name);

     frame_free(t);

     free(t);
}


void
tag_free(struct Scr33n *s)
{
     struct Tag *t;

     TAILQ_FOREACH(t, &s->tags, next)
     {
          TAILQ_REMOVE(&s->tags, t, next);
          tag_remove(t);
     }
}
