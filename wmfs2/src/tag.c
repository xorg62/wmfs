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
     struct tag *t;
     XSetWindowAttributes at =
     {
          .background_pixel  = THEME_DEFAULT->frame_bg,
          .override_redirect = true,
          .background_pixmap = ParentRelative,
          .event_mask        = BARWIN_MASK
     };

     t = xcalloc(1, sizeof(struct tag));

     t->screen = s;
     t->name   = xstrdup(name);
     t->flags  = 0;
     t->sel    = NULL;

     /* Frame window */
     t->frame = XCreateWindow(W->dpy, W->root,
                              s->ugeo.x, s->ugeo.y,
                              s->ugeo.w, s->ugeo.h,
                              0, CopyFromParent,
                              InputOutput,
                              CopyFromParent,
                              (CWOverrideRedirect | CWBackPixmap
                               | CWBackPixel | CWEventMask),
                              &at);

     SLIST_INIT(&t->clients);

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(struct screen *s, struct tag *t)
{
     struct client *c;

     /* Unmap previous tag's frame */
     WIN_STATE(s->seltag->frame, Unmap);
     SLIST_FOREACH(c, &s->seltag->clients, tnext)
          ewmh_set_wm_state(c->win, IconicState);

     /*
      * Map selected tag's frame, only if there is
      * clients in t
      */
     if(!SLIST_EMPTY(&t->clients))
     {
          WIN_STATE(t->frame, Map);
          SLIST_FOREACH(c, &t->clients, tnext)
               ewmh_set_wm_state(c->win, NormalState);
     }

     s->seltag = t;

     client_focus(t->sel);

     infobar_elem_screen_update(s, ElemTag);
}

/* Set t to NULL to untag c from c->tag */
void
tag_client(struct tag *t, struct client *c)
{
     /* Remove client from its previous tag */
     if(c->tag)
     {
          if(c->tag == t)
               return;

          layout_split_arrange_closed(c);

          SLIST_REMOVE(&c->tag->clients, c, client, tnext);

          /* TODO: Focus next client */
          if(c->tag->sel == c)
               c->tag->sel = NULL;
     }

     /*
      * Case of client removing: umap frame if empty
      */
     if(!t)
     {
          /* Unmap frame if tag is now empty */
          if(SLIST_EMPTY(&c->tag->clients))
               WIN_STATE(c->tag->frame, Unmap);

          return;
     }

     /* Map frame if tag was empty */
     if(SLIST_EMPTY(&t->clients))
          WIN_STATE(t->frame, Map);

     c->tag = t;

     /* Reparent client win in frame win */
     XReparentWindow(W->dpy, c->win, t->frame, 0, 0);

     layout_split_integrate(c, t->sel);

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
     free(t->name);

     XDestroyWindow(W->dpy, t->frame);

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
