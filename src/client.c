/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h>

#include "client.h"
#include "config.h"
#include "util.h"
#include "barwin.h"
#include "ewmh.h"
#include "layout.h"
#include "draw.h"

#define CLIENT_MOUSE_MOD Mod1Mask

#define CLIENT_RESIZE_DIR(D)                          \
void uicb_client_resize_##D(Uicb cmd)                 \
{                                                     \
     if(W->client)                                    \
          client_fac_resize(W->client, D, ATOI(cmd)); \
}

#define CLIENT_ACTION_DIR(A, D)                                \
void uicb_client_##A##_##D(Uicb cmd)                           \
{                                                              \
     (void)cmd;                                                \
     struct client *c;                                         \
     if(W->client && (c = client_next_with_pos(W->client, D))) \
          client_##A(c);                                       \
}

#define CLIENT_ACTION_LIST(A, L)                  \
void uicb_client_##A##_##L(Uicb cmd)              \
{                                                 \
     (void)cmd;                                   \
     struct client *c;                            \
     if(W->client && (c = client_##L(W->client))) \
          client_##A(c);                          \
}

/* uicb_client_resize_dir() */
CLIENT_RESIZE_DIR(Right)
CLIENT_RESIZE_DIR(Left)
CLIENT_RESIZE_DIR(Top)
CLIENT_RESIZE_DIR(Bottom)

/* uicb_client_focus_dir() */
CLIENT_ACTION_DIR(focus, Right)
CLIENT_ACTION_DIR(focus, Left)
CLIENT_ACTION_DIR(focus, Top)
CLIENT_ACTION_DIR(focus, Bottom)

/* uicb_client_swapsel_dir() */
#define client_swapsel(c) client_swap(W->client, c)
CLIENT_ACTION_DIR(swapsel, Right)
CLIENT_ACTION_DIR(swapsel, Left)
CLIENT_ACTION_DIR(swapsel, Top)
CLIENT_ACTION_DIR(swapsel, Bottom)

/* uicb_client_focus_next/prev() */
CLIENT_ACTION_LIST(focus, next)
CLIENT_ACTION_LIST(focus, prev)

/* uicb_client_swapsel_next/prev() */
CLIENT_ACTION_LIST(swapsel, next)
CLIENT_ACTION_LIST(swapsel, prev)

/** Send a ConfigureRequest event to the struct client
 * \param c struct client pointer
*/
void
client_configure(struct client *c)
{
     XConfigureEvent ev =
     {
          .type              = ConfigureNotify,
          .event             = c->win,
          .window            = c->win,
          .x                 = c->geo.x,
          .y                 = c->geo.y,
          .width             = c->geo.w,
          .height            = c->geo.h,
          .above             = None,
          .border_width      = 0,
          .override_redirect = 0
     };

     XSendEvent(W->dpy, c->win, False, StructureNotifyMask, (XEvent *)&ev);
     XSync(W->dpy, False);
}

struct client*
client_gb_win(Window w)
{
     struct client *c = SLIST_FIRST(&W->h.client);

     while(c && c->win != w)
          c = SLIST_NEXT(c, next);

     return c;
}

struct client*
client_gb_pos(struct tag *t, int x, int y)
{
     struct client *c = SLIST_FIRST(&t->clients);

     while(c)
     {
          if(INAREA(x, y, c->geo))
               return c;

          c = SLIST_NEXT(c, tnext);
     }

     return NULL;
}

/** Get client left/right/top/bottom of selected client
  *\param bc Base client
  *\param pos Position
  *\return Client found or NULL
*/
struct client*
client_next_with_pos(struct client *bc, Position p)
{
     struct client *c;
     int x, y;
     static const char scanfac[PositionLast] = { +10, -10, 0, 0 };
     Position ip = Bottom - p;

     /*
      * Set start place of pointer (edge with position
      * of base client) for faster scanning.
      */
     x = bc->geo.x + ((p == Right)  ? bc->geo.w : 0);
     y = bc->geo.y + ((p == Bottom) ? bc->geo.h : 0);
     y += ((LDIR(p))  ? (bc->geo.h >> 1) : 0);
     x += ((p > Left) ? (bc->geo.w >> 1) : 0);

     /* Scan in right direction to next(p) physical client */
     while((c = client_gb_pos(bc->tag, x, y)) == bc)
     {
          x += scanfac[p];
          y += scanfac[ip];
     }

     return c;
}

void
client_swap(struct client *c1, struct client *c2)
{
     struct tag *t;
     struct geo g;

     if(c1 == c2 || !c1 || !c2)
          return;

     t = c1->tag;
     g = c1->geo;

     swap_ptr((void**)&c1->screen, (void**)&c2->screen);

     tag_client(c2->tag, c1);
     tag_client(t, c2);

     client_moveresize(c1, c2->geo);
     client_moveresize(c2, g);
}

static void
client_grabbuttons(struct client *c, bool focused)
{
     XUngrabButton(W->dpy, AnyButton, AnyModifier, c->win);

     if(focused)
     {
          int i = 0;

          while(i++ != Button5)
          {
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | LockMask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | W->numlockmask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | LockMask | W->numlockmask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          }

          return;
     }

     XGrabButton(W->dpy, AnyButton, AnyModifier, c->win, False,
               ButtonMask, GrabModeAsync, GrabModeSync, None, None);

}

static inline void
client_draw_bord(struct client *c)
{
     struct geo g = { 0, 0, c->screen->ugeo.w, c->screen->ugeo.h };

     draw_rect(c->tag->frame, g, THEME_DEFAULT->client_n.bg);

     /* Selected client's border */
     if(W->client)
          draw_rect(W->client->tag->frame, W->client->tag->sel->geo, THEME_DEFAULT->client_s.bg);
}


void
client_focus(struct client *c)
{
     /* Unfocus selected */
     if(W->client && W->client != c)
          client_grabbuttons(W->client, false);

     /* Focus c */
     if((W->client = c))
     {
          c->tag->sel = c;

          client_draw_bord(c);
          client_grabbuttons(c, true);

          XSetInputFocus(W->dpy, c->win, RevertToPointerRoot, CurrentTime);
     }
     else
     {
          W->client = W->screen->seltag->sel = NULL;
          XSetInputFocus(W->dpy, W->root, RevertToPointerRoot, CurrentTime);
     }
}

/** Get a client name
 * \param c struct client pointer
*/
void
client_get_name(struct client *c)
{
     Atom rt;
     int rf;
     unsigned long ir, il;

     /* This one instead XFetchName for utf8 name support */
     if(XGetWindowProperty(W->dpy, c->win, ATOM("_NET_WM_NAME"), 0, 4096,
                    False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, (unsigned char**)&c->title) != Success)
          XGetWindowProperty(W->dpy, c->win, ATOM("WM_NAME"), 0, 4096,
                             False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, (unsigned char**)&c->title);

     /* Still no title... */
     if(!c->title)
          XFetchName(W->dpy, c->win, &(c->title));
}

/** Close a client
 * \param c struct client pointer
*/
void
client_close(struct client *c)
{
     int proto;
     XEvent ev;
     Atom *atom = NULL;

     /* Event will call client_remove */
     if(XGetWMProtocols(W->dpy, c->win, &atom, &proto) && atom)
     {
          while(proto--)
               if(atom[proto] == ATOM("WM_DELETE_WINDOW"))
               {
                    ev.type = ClientMessage;
                    ev.xclient.window = c->win;
                    ev.xclient.message_type = ATOM("WM_PROTOCOLS");
                    ev.xclient.format = 32;
                    ev.xclient.data.l[0] = ATOM("WM_DELETE_WINDOW");
                    ev.xclient.data.l[1] = CurrentTime;

                    XSendEvent(W->dpy, c->win, False, NoEventMask, &ev);
                    XFree(atom);

                    return;
               }
     }

     XKillClient(W->dpy, c->win);
}

void
uicb_client_close(Uicb cmd)
{
     (void)cmd;

     if(W->client)
          client_close(W->client);
}

struct client*
client_new(Window w, XWindowAttributes *wa)
{
     struct client *c = xcalloc(1, sizeof(struct client));

     /* C attributes */
     c->win    = w;
     c->screen = W->screen;
     c->flags  = 0;
     c->tag    = NULL;

     /* struct geometry */
     c->geo.x = wa->x;
     c->geo.y = wa->y;
     c->geo.w = wa->width;
     c->geo.h = wa->height;
     c->tgeo = c->wgeo = c->geo;

     /* Set tag */
     tag_client(W->screen->seltag, c);

     /* X window attributes */
     XSelectInput(W->dpy, w, EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask);
     XSetWindowBorderWidth(W->dpy, w, 0);
     client_grabbuttons(c, false);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.client, c, next);

     /* Map */
     WIN_STATE(w, Map);
     ewmh_set_wm_state(w, NormalState);

     client_get_name(c);
     client_focus(c);
     client_configure(c);

     return c;
}

void
client_moveresize(struct client *c, struct geo g)
{
     int bord = THEME_DEFAULT->client_border_width;

     c->geo = g;

     /* Window geo */
     c->wgeo.x = g.x + bord;
     c->wgeo.y = g.y + bord ;
     c->wgeo.w = g.w - (bord << 1);
     c->wgeo.h = g.h - (bord << 1);

     XMoveResizeWindow(W->dpy, c->win,
                       c->wgeo.x, c->wgeo.y,
                       c->wgeo.w, c->wgeo.h);

     client_draw_bord(c);
     client_configure(c);
}

void
client_maximize(struct client *c)
{
     c->geo = c->tag->screen->ugeo;

     c->geo.x = c->geo.y = 0; /* Frame x/y, not screen geo */
     c->geo.w = c->tag->screen->ugeo.w;
     c->geo.h = c->tag->screen->ugeo.h;

     client_moveresize(c, c->geo);
}

void
client_fac_resize(struct client *c, Position p, int fac)
{
     struct client *gc = client_next_with_pos(c, p);
     Position rp = RPOS(p);

     if(!gc || gc->screen != c->screen)
          return;

     /* Check futur size/pos */
     if(!client_fac_geo(c, p, fac)
        || !client_fac_geo(gc, rp, -fac)
        || !client_fac_check_row(c, p, fac)
        || !client_fac_check_row(gc, rp, -fac))
          return;


     /* Simple resize with only c & gc */
     if(GEO_CHECK2(c->geo, gc->geo, p))
     {
          client_moveresize(c, c->tgeo);
          client_moveresize(gc, gc->tgeo);
     }
     /* Resize with row parents */
     else
     {
          client_fac_arrange_row(c, p, fac);
          client_fac_arrange_row(gc, rp, -fac);
     }
}

void
client_remove(struct client *c)
{
     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);
     XReparentWindow(W->dpy, c->win, W->root, c->geo.x, c->geo.y);

     /* Remove from global client list */
     SLIST_REMOVE(&W->h.client, c, client, next);

     tag_client(NULL, c);

     ewmh_set_wm_state(c->win, WithdrawnState);

     XUngrabServer(W->dpy);
     XSync(W->dpy, False);
     XSetErrorHandler(wmfs_error_handler);

     free(c);
}

void
client_free(void)
{
     FREE_LIST(client, W->h.client);
}
