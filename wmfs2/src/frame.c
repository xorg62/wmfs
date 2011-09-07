/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "frame.h"
#include "barwin.h"
#include "tag.h"
#include "util.h"

struct Frame*
frame_new(struct Tag *t)
{
     struct Geo g = t->screen->ugeo;
     struct Frame *f = xcalloc(1, sizeof(struct Frame));
     XSetWindowAttributes at =
     {
          .override_redirect = True,
          .background_pixmap = ParentRelative,
          .event_mask = (BARWIN_MASK | BARWIN_ENTERMASK)
     };

     f->tag = t;
     f->geo = g;

     f->win = XCreateWindow(W->dpy, W->root, g.x, g.y, g.w, g.h, 0, W->xdepth,
               CopyFromParent, DefaultVisual(W->dpy, W->xscreen),
               (CWOverrideRedirect | CWEventMask), &at);

     SLIST_INIT(&f->clients);

     SLIST_INSERT_HEAD(&t->frames, f, next);
}

static void
frame_remove(struct Frame *f)
{
     SLIST_REMOVE(&f->tag->frames, f, Frame, next);
     XDestroyWindow(W->dpy, f->win);

     /* frame_arrange(f->tag); */
     free(f);
}

void
frame_free(struct Tag *t)
{
     struct Frame *f;

     SLIST_FOREACH(f, &t->frames, next)
          frame_remove(f);
}

void
frame_client(struct Frame *f, struct Client *c)
{
     /* Remove client from its previous frame */
     if(c->frame)
          SLIST_REMOVE(&c->frame->clients, c, Client, fnext);

     /* Adjust tag with frame's one */
     if(f->tag != c->tag)
          tag_client(f->tag, c);

     XReparentWindow(W->dpy, c->win, f->win, 1, 1);

     /* XReparentWindow(W->dpy, c->win, c->titlebar->win, 1, 1); */

     SLIST_INSERT_HEAD(&f->clients, c, next);
}

void
frame_update(struct Frame *f)
{

}








