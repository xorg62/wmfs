/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "mouse.h"
#include "client.h"
#include "draw.h"

#define _REV_BORDER()                                        \
     do {                                                    \
          SLIST_FOREACH(gc, &c->tag->clients, tnext)         \
               draw_reversed_rect(W->root, &gc->tgeo);       \
     } while(/* CONSTCOND */ 0);
static void
mouse_resize(struct client *c)
{
     struct client *gc;
     XEvent ev;
     Window w;
     int d, u, ox, oy, ix, iy;

     XQueryPointer(W->dpy, W->root, &w, &w, &ox, &oy, &d, &d, (uint *)&u);
     XGrabServer(W->dpy);

     _REV_BORDER();

     if(c->flags & CLIENT_TABBED && !(c->flags & CLIENT_TABMASTER))
          c = c->tabmaster;

     ix = ox;
     iy = oy;

     do
     {
          XMaskEvent(W->dpy, MouseMask | SubstructureRedirectMask, &ev);

          if(ev.type != MotionNotify)
               continue;

          _REV_BORDER();

          if(ix >= c->geo.x + (c->geo.w >> 1))
               _fac_resize(c, Right, ev.xmotion.x_root - ox);
          else
               _fac_resize(c, Left, ox - ev.xmotion.x_root);

          if(iy >= c->geo.y + (c->geo.h >> 1))
               _fac_resize(c, Bottom, ev.xmotion.y_root - oy);
          else
               _fac_resize(c, Top, oy - ev.xmotion.y_root);

          ox = ev.xmotion.x_root;
          oy = ev.xmotion.y_root;

          _REV_BORDER();

          XSync(W->dpy, false);

     } while(ev.type != ButtonRelease);

     _REV_BORDER();

     client_apply_tgeo(c->tag);
     layout_save_set(c->tag);

     XUngrabServer(W->dpy);
}

#define _REV_SBORDER(c) draw_reversed_rect(W->root, &(c)->geo);
void
mouse_move(struct client *c)
{
     struct client *c2 = NULL, *last = c;
     XEvent ev;
     Window w;
     int d, u;

     XGrabServer(W->dpy);

     if(c->flags & CLIENT_TABBED && !(c->flags & CLIENT_TABMASTER))
          c = c->tabmaster;

     _REV_SBORDER(c);

     do
     {
          XMaskEvent(W->dpy, MouseMask | SubstructureRedirectMask, &ev);

          if(ev.type != MotionNotify)
               continue;

          XQueryPointer(W->dpy, W->root, &w, &w, &d, &d, &d, &d, (uint *)&u);

          if(!(c2 = client_gb_win(w)))
               if(!(c2 = client_gb_frame(w)))
                    c2 = client_gb_titlebar(w);

          if(c2 && c2 != last)
          {
               _REV_SBORDER(last);
               _REV_SBORDER(c2);
               last = c2;
          }

          XSync(W->dpy, false);

     } while(ev.type != ButtonRelease);

     if(c2 && c2 != c)
     {
          _REV_SBORDER(c2);
          client_swap2(c, c2);
     }

     XUngrabServer(W->dpy);
}

void
uicb_mouse_resize(Uicb cmd)
{
     (void)cmd;

     if(mouse_check_client(W->client))
          if(W->client)
               mouse_resize(W->client);
}

void
uicb_mouse_move(Uicb cmd)
{
     (void)cmd;

     if(mouse_check_client(W->client))
          if(W->client)
               mouse_move(W->client);
}
