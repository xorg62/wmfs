/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "mouse.h"
#include "barwin.h"
#include "client.h"
#include "draw.h"

#define _REV_SBORDER(c) draw_reversed_rect(W->root, c, false);

#define _REV_BORDER()                                   \
     do {                                               \
          FOREACH_NFCLIENT(gc, &c->tag->clients, tnext) \
               draw_reversed_rect(W->root, gc, true);   \
     } while(/* CONSTCOND */ 0);

static void
mouse_resize(struct client *c)
{
     struct client *gc;
     XEvent ev;
     Window w;
     int d, u, ox, oy, ix, iy;
     int mx, my;

     XQueryPointer(W->dpy, W->root, &w, &w, &ox, &oy, &d, &d, (unsigned int *)&u);
     XGrabServer(W->dpy);

     if(c->flags & CLIENT_FREE)
     {
          _REV_SBORDER(c);
     }
     else
          _REV_BORDER();

     if(c->flags & CLIENT_TABBED && !(c->flags & CLIENT_TABMASTER))
          c = c->tabmaster;

     ix = ox;
     iy = oy;

     c->flags |= CLIENT_MOUSE;

     do
     {
          XMaskEvent(W->dpy, MouseMask | SubstructureRedirectMask, &ev);

          if(ev.type != MotionNotify)
               continue;

          mx = ev.xmotion.x_root;
          my = ev.xmotion.y_root;

          if(c->flags & CLIENT_FREE)
          {
               _REV_SBORDER(c);

               mx -= c->screen->ugeo.x;
               my -= c->screen->ugeo.y;

               c->geo.w = ((mx - c->geo.x <= c->sizeh[MINW] + c->border + c->border)
                           ? c->sizeh[MINW] + c->border + c->border
                           : mx - c->geo.x);
               c->geo.h = ((my - c->geo.y <= (c->sizeh[MINH] + c->tbarw + c->border))
                           ? c->sizeh[MINH] + c->tbarw + c->border
                           : my - c->geo.y);

               client_geo_hints(&c->geo, (int*)c->sizeh);

               /* For border preview cohesion */
               c->geo.h += c->tbarw + c->border;
               c->geo.w += c->border + c->border;

               _REV_SBORDER(c);
          }
          else
          {
               _REV_BORDER();

               if(ix >= c->rgeo.x + (c->geo.w >> 1))
                    _fac_resize(c, Right, mx - ox);
               else
                    _fac_resize(c, Left, ox - mx);

               if(iy >= c->rgeo.y + (c->geo.h >> 1))
                    _fac_resize(c, Bottom, my - oy);
               else
                    _fac_resize(c, Top, oy - my);

               ox = mx;
               oy = my;

               _REV_BORDER();
          }

          XSync(W->dpy, false);

     } while(ev.type != ButtonRelease);

     if(c->flags & CLIENT_FREE)
     {
          _REV_SBORDER(c);
          client_moveresize(c, &c->geo);
     }
     else
     {
          _REV_BORDER();
          client_apply_tgeo(c->tag);
          layout_save_set(c->tag);
     }

     c->flags &= ~CLIENT_MOUSE;

     XUngrabServer(W->dpy);
}

static struct tag*
mouse_drag_tag(struct client *c, Window w)
{
     struct barwin *b;
     struct tag *t = NULL;
     Window rw;
     int d, u;

     XQueryPointer(W->dpy, w, &rw, &rw, &d, &d, &d, &d, (uint *)&u);

     SLIST_FOREACH(b, &W->h.barwin, next)
          if(b->win == rw
             && (t = (struct tag*)b->ptr)
             && t != c->tag)
               return t;

     return NULL;
}

void
mouse_move(struct client *c, void (*func)(struct client*, struct client*))
{
     struct client *c2 = NULL, *last = c;
     struct tag *t = NULL;
     XEvent ev;
     Window w;
     int d, u, ox, oy;
     int ocx, ocy;

     ocx = c->geo.x;
     ocy = c->geo.y;

     if(c->flags & CLIENT_TABBED && !(c->flags & CLIENT_TABMASTER))
          c = c->tabmaster;

     XQueryPointer(W->dpy, W->root, &w, &w, &ox, &oy, &d, &d, (uint *)&u);

     _REV_SBORDER(c);

     c->flags |= CLIENT_MOUSE;

     do
     {
          XMaskEvent(W->dpy, MouseMask | SubstructureRedirectMask, &ev);

          if(ev.type != MotionNotify)
               continue;

          if(!func && c->flags & CLIENT_FREE)
          {
               _REV_SBORDER(c);

               c->geo.x = (ocx + (ev.xmotion.x_root - ox));
               c->geo.y = (ocy + (ev.xmotion.y_root - oy));

               _REV_SBORDER(c);
          }
          else
          {
               XQueryPointer(W->dpy, W->root, &w, &w, &d, &d, &d, &d, (uint *)&u);

               if(!(c2 = client_gb_win(w)))
                    if(!(c2 = client_gb_frame(w)))
                         c2 = client_gb_titlebar(w);

               if(c2)
               {
                    if(c2 != last)
                    {
                         _REV_SBORDER(last);
                         _REV_SBORDER(c2);
                         last = c2;
                    }
               }
               else
                    t = mouse_drag_tag(c, w);
          }

          XSync(W->dpy, false);

     } while(ev.type != ButtonRelease);

     if(c2)
          func(c, c2);
     else if(t && t != (struct tag*)c)
          tag_client(t, c);
     else
     {
          _REV_SBORDER(c);

          /* No func mean free client resize */
          if(!func)
               client_moveresize(c, &c->geo);
     }

     c->flags &= ~CLIENT_MOUSE;
}

void
uicb_mouse_resize(Uicb cmd)
{
     (void)cmd;

     if(W->client && mouse_check_client(W->client))
          mouse_resize(W->client);
}

void
uicb_mouse_move(Uicb cmd)
{
     (void)cmd;

     if(W->client && mouse_check_client(W->client))
          mouse_move(W->client, (W->client->flags & CLIENT_FREE ? NULL : client_swap2));
}

void
uicb_mouse_tab(Uicb cmd)
{
     (void)cmd;

     if(W->client && mouse_check_client(W->client))
          mouse_move(W->client, _client_tab);
}
