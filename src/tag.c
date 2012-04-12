/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h> /* IconicState / NormalState */

#include "tag.h"
#include "util.h"
#include "infobar.h"
#include "client.h"
#include "config.h"
#include "barwin.h"
#include "ewmh.h"
#include "layout.h"

struct tag*
tag_new(struct screen *s, char *name)
{
     struct tag *t, *l;

     t = xcalloc(1, sizeof(struct tag));

     t->screen = s;
     t->flags  = 0;
     t->id     = 0;
     t->sel    = NULL;
     t->prev   = NULL;

     if((l = TAILQ_LAST(&s->tags, tsub)))
          t->id = l->id + 1;

     if(!name || !strlen(name))
          xasprintf(&t->name, "%d", t->id + 1);
     else
          t->name = xstrdup(name);

     SLIST_INIT(&t->clients);
     TAILQ_INIT(&t->sets);

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(struct screen *s, struct tag *t)
{
     struct client *c;

     if(t == s->seltag && TAILQ_NEXT(TAILQ_FIRST(&s->tags), next))
          t = t->prev;

     if(!t)
          t = TAILQ_FIRST(&s->tags);

     t->prev = s->seltag;
     s->seltag = t;

     /* Move clients if they ignore tags */
     SLIST_FOREACH(c, &W->h.client, next)
          if (c->flags & CLIENT_IGNORE_TAG)
               tag_client(c->screen->seltag, c);
     clients_arrange_map();

     if(!SLIST_EMPTY(&t->clients) && !(W->flags & WMFS_SCAN))
          client_focus( client_tab_next(t->sel));

     t->flags &= ~TAG_URGENT;

     infobar_elem_screen_update(s, ElemTag);

     ewmh_update_wmfs_props();
}

/* Set t to NULL to untag c from c->tag */
void
tag_client(struct tag *t, struct client *c)
{
     /* Remove client from its previous tag */
     if(c->tag && !(c->flags & CLIENT_RULED))
     {
          if(c->tag == t)
               return;

          if(!(c->flags & (CLIENT_IGNORE_LAYOUT | CLIENT_FREE)))
               layout_split_arrange_closed(c);

          if(!(c->flags & CLIENT_REMOVEALL))
          {
               SLIST_REMOVE(&c->tag->clients, c, client, tnext);

               if(c->tag->sel == c || W->client == c)
                    client_focus( client_tab_next( client_next(c)));
          }
     }

     c->flags &= ~CLIENT_RULED;

     /* Client remove */
     if(!t)
     {
          infobar_elem_screen_update(c->screen, ElemTag);
          return;
     }

     c->prevtag = c->tag;
     c->tag = t;
     c->screen = t->screen;

     client_update_props(c, CPROP_LOC);

     SLIST_INSERT_HEAD(&t->clients, c, tnext);

     infobar_elem_screen_update(c->screen, ElemTag);

     if(c->flags & CLIENT_TABMASTER && c->prevtag)
     {
          struct client *cc;

          SLIST_FOREACH(cc, &c->prevtag->clients, tnext)
               if(cc->tabmaster == c)
               {
                    cc->flags |= CLIENT_IGNORE_LAYOUT;
                    tag_client(t, cc);
               }
     }

     layout_client(c);

     if(t != c->screen->seltag || c->flags & CLIENT_TABBED)
          client_unmap(c);
}

void
uicb_tag_set(Uicb cmd)
{
     int i = 0, n = ATOI(cmd);
     struct tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(i++ == n)
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

void
uicb_tag_client(Uicb cmd)
{
     struct tag *t;
     int id = ATOI(cmd);

     if(W->client && (t = tag_gb_id(W->screen, id)))
          tag_client(t, W->client);
}

void
uicb_tag_client_and_set(Uicb cmd)
{
    uicb_tag_client(cmd);
    uicb_tag_set(cmd);
}

void
uicb_tag_move_client_next(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if(!W->client)
          return;

     if((t = TAILQ_NEXT(W->screen->seltag, next)))
          tag_client(t, W->client);
     else if( /* CIRCULAR OPTION */ 1)
          tag_client(TAILQ_FIRST(&W->screen->tags), W->client);
}

void
uicb_tag_move_client_prev(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if(!W->client)
          return;

     if((t = TAILQ_PREV(W->screen->seltag, tsub, next)))
          tag_client(t, W->client);
     else if( /* CIRCULAR OPTION */ 1)
          tag_client(TAILQ_LAST(&W->screen->tags, tsub), W->client);
}

void
uicb_tag_click(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if((t = (struct tag*)W->last_clicked_barwin->ptr)
        && t->screen == W->screen)
          tag_screen(W->screen, t);
}

static void
tag_remove(struct tag *t)
{
     TAILQ_REMOVE(&t->screen->tags, t, next);

     free(t->name);

     layout_free_set(t);

     free(t);
}

void
uicb_tag_new(Uicb cmd)
{
     struct screen *s = W->screen;
     struct infobar *i;

     tag_new(s, (char*)cmd);

     s->flags |= SCREEN_TAG_UPDATE;

     SLIST_FOREACH(i, &s->infobars, next)
          infobar_elem_reinit(i);

     s->flags ^= SCREEN_TAG_UPDATE;
}

void
uicb_tag_del(Uicb cmd)
{
     struct infobar *i;
     struct tag *t = W->screen->seltag;
     (void)cmd;

     if(SLIST_EMPTY(&t->clients)
        && TAILQ_NEXT(TAILQ_FIRST(&W->screen->tags), next))
     {
          tag_screen(W->screen, TAILQ_NEXT(t, next));
          tag_remove(t);

          W->screen->flags |= SCREEN_TAG_UPDATE;

          SLIST_FOREACH(i, &W->screen->infobars, next)
               infobar_elem_reinit(i);

          W->screen->flags ^= SCREEN_TAG_UPDATE;
     }
}

void
tag_free(struct screen *s)
{
     while(!TAILQ_EMPTY(&s->tags))
          tag_remove(TAILQ_FIRST(&s->tags));
}
