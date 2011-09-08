/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h>

#include "wmfs.h"
#include "frame.h"
#include "barwin.h"
#include "tag.h"
#include "util.h"
#include "config.h"

struct frame*
frame_new(struct tag *t)
{
     struct frame *f = xcalloc(1, sizeof(struct frame));
     XSetWindowAttributes at =
     {
          .background_pixel  = THEME_DEFAULT->frame_bg,
          .override_redirect = true,
          .background_pixmap = ParentRelative,
          .event_mask        = BARWIN_MASK
     };

     f->tag = t;
     f->geo = t->screen->ugeo;
     t->frame = f;

     f->win = XCreateWindow(W->dpy, W->root,
                            f->geo.x, f->geo.y,
                            f->geo.w, f->geo.h,
                            0, CopyFromParent,
                            InputOutput,
                            CopyFromParent,
                            (CWOverrideRedirect | CWBackPixmap | CWBackPixel | CWEventMask),
                            &at);

     SLIST_INIT(&f->clients);
     SLIST_INSERT_HEAD(&t->frames, f, next);
}

static void
frame_remove(struct frame *f)
{
     SLIST_REMOVE(&f->tag->frames, f, frame, next);
     XDestroyWindow(W->dpy, f->win);

     /* frame_arrange(f->tag); */
     free(f);
}

void
frame_free(struct tag *t)
{
     struct frame *f;

     SLIST_FOREACH(f, &t->frames, next)
          frame_remove(f);
}

void
frame_client(struct frame *f, struct client *c)
{
     /* Remove client from its previous frame */
     if(c->frame)
     {
          if(c->frame == f)
               return;

          SLIST_REMOVE(&c->frame->clients, c, client, fnext);
     }

     /* Case of client remove, f = NULL */
     if(!f)
     {
          if(c->frame && FRAME_EMPTY(c->frame))
               frame_unmap(c->frame);

          XReparentWindow(W->dpy, c->win, W->root, c->geo.x, c->geo.y);

          return;
     }

     XReparentWindow(W->dpy, c->win, f->win,
                     THEME_DEFAULT->client_border_width,
                     THEME_DEFAULT->client_titlebar_width);

     XResizeWindow(W->dpy, c->win,
                   f->geo.w - (THEME_DEFAULT->client_border_width << 1) - 1,
                   f->geo.h - (THEME_DEFAULT->client_titlebar_width
                        + THEME_DEFAULT->client_border_width >> 1));

     /* XReparentWindow(W->dpy, c->win, c->titlebar->win, 1, 1); */

     /*split_integrate(f, c) */

     SLIST_INSERT_HEAD(&f->clients, c, fnext);

     frame_update(f);
}

void
frame_map(struct frame *f)
{
     struct client *c;

     WIN_STATE(f->win, Map);

     SLIST_FOREACH(c, &f->clients, fnext)
          ewmh_set_wm_state(c->win, NormalState);
}

void
frame_unmap(struct frame *f)
{
     struct client *c;

     WIN_STATE(f->win, Unmap);

     SLIST_FOREACH(c, &f->clients, fnext)
          ewmh_set_wm_state(c->win, IconicState);
}

void
frame_update(struct frame *f)
{
     if(FRAME_EMPTY(f))
          return;

     /* Resize frame */
     /* TODO: frame_arrange or something */
     XMoveResizeWindow(W->dpy,
               f->win,
               f->geo.x,
               f->geo.y,
               f->geo.w,
               f->geo.h);

     frame_map(f);
}








