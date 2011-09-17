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

#define CLIENT_MOUSE_MOD Mod1Mask

#define CLIENT_RESIZE_DIR(d)                          \
void uicb_client_resize_##d(Uicb cmd)                 \
{                                                     \
     if(W->client)                                    \
          client_fac_resize(W->client, d, ATOI(cmd)); \
}

/* uicb_client_resize_dir() */
CLIENT_RESIZE_DIR(Right)
CLIENT_RESIZE_DIR(Left)
CLIENT_RESIZE_DIR(Top)
CLIENT_RESIZE_DIR(Bottom)

/** Send a ConfigureRequest event to the struct client
 * \param c struct client pointer
*/
void
client_configure(struct client *c)
{
     XConfigureEvent ev;

     ev.type              = ConfigureNotify;
     ev.event             = c->win;
     ev.window            = c->win;
     ev.x                 = c->geo.x;
     ev.y                 = c->geo.y;
     ev.width             = c->geo.w;
     ev.height            = c->geo.h;
     ev.above             = None;
     ev.border_width      = 0;
     ev.override_redirect = 0;

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

static struct client*
client_gb_pos(struct tag *t, int x, int y)
{
     struct client *c;

     SLIST_FOREACH(c, &t->clients, tnext)
          if(INAREA(x, y, c->geo))
               return c;

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
     const static char scanfac[4][2] =
     {
          { 2,  0 }, { -2, 0 }, /* Right, Left   */
          { 0, -2 }, {  0, 2 }  /* Top,   Bottom */
     };

     /*
      * Set start place of pointer (edge of base client)
      * for faster scanning
      */
     x = bc->geo.x + ((p == Right)  ? bc->geo.w : 0);
     y = bc->geo.y + ((p == Bottom) ? bc->geo.h : 0);
     y += ((LDIR(p))  ? (bc->geo.h >> 1) : 0);
     x += ((p > Left) ? (bc->geo.w >> 1) : 0);

     /* Scan in right direction to next(p) physical client */

     while((c = client_gb_pos(bc->tag, x, y)) == bc)
     {
          x += scanfac[p][0];
          y += scanfac[p][1];
     }

     return c;
}

/** Map a client
 * \param c struct client pointer
 */
void
client_map(struct client *c)
{
     XMapWindow(W->dpy, c->win);
}

/** Unmap a client
 * \param c struct client pointer
 */
void
client_unmap(struct client *c)
{
     XUnmapWindow(W->dpy, c->win);
}

static void
client_grabbuttons(struct client *c, bool focused)
{
     wmfs_numlockmask();

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

void
client_focus(struct client *c)
{
     /* Unfocus selected */
     if(W->client && W->client != c)
     {
          XSetWindowBorder(W->dpy, W->client->win, THEME_DEFAULT->client_n.bg);

          client_grabbuttons(W->client, false);
     }

     /* Focus c */
     if((W->client = c))
     {
          c->tag->sel = c;

          XSetWindowBorder(W->dpy, c->win, THEME_DEFAULT->client_s.bg);

          client_grabbuttons(c, true);

          XSetInputFocus(W->dpy, c->win, RevertToPointerRoot, CurrentTime);
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
     XEvent ev;
     Atom *atom = NULL;
     int proto;

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
                    ev.xclient.data.l[2] = 0;
                    ev.xclient.data.l[3] = 0;
                    ev.xclient.data.l[4] = 0;

                    XSendEvent(W->dpy, c->win, False, NoEventMask, &ev);

                    XFree(atom);

                    return;
               }

          XKillClient(W->dpy, c->win);
     }
     else
          XKillClient(W->dpy, c->win);
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
     c->tgeo = c->geo;

     /* Set tag */
     tag_client(W->screen->seltag, c);

     /* X window attributes */
     XSelectInput(W->dpy, w, EnterWindowMask | FocusChangeMask
                             | PropertyChangeMask | StructureNotifyMask);
     XSetWindowBorder(W->dpy, w, THEME_DEFAULT->client_n.bg);
     XSetWindowBorderWidth(W->dpy, w, THEME_DEFAULT->client_border_width);
     client_grabbuttons(c, false);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.client, c, next);

     /* Map */
     WIN_STATE(c->win, Map);
     ewmh_set_wm_state(c->win, NormalState);

     client_configure(c);

     return c;
}

void
client_moveresize(struct client *c, struct geo g)
{
     c->geo = g;

     XMoveResizeWindow(W->dpy, c->win, g.x, g.y, g.w, g.h);
}

void
client_maximize(struct client *c)
{
     c->geo = c->tag->screen->ugeo;

     c->geo.x = c->geo.y = 0; /* Frame x/y, not screen geo */
     c->geo.w = c->tag->screen->ugeo.w - (THEME_DEFAULT->client_border_width << 1);
     c->geo.h = c->tag->screen->ugeo.h - (THEME_DEFAULT->client_border_width << 1);

     client_moveresize(c, c->geo);
}

void
client_fac_resize(struct client *c, Position p, int fac)
{
     struct client *gc = client_next_with_pos(c, p);

     if(!gc || gc->screen != c->screen)
          return;

     /* Check futur size/pos */
     if(!client_fac_geo(c, p, fac)
         || !client_fac_geo(gc, RPOS(p), -fac)
         || !client_fac_check_row(c, p, fac)
         || !client_fac_check_row(gc, RPOS(p), -fac))
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
          client_fac_arrange_row(gc, RPOS(p), -fac);
     }
}

void
client_remove(struct client *c)
{
     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);
     XReparentWindow(W->dpy, c->win, W->root, c->geo.x, c->geo.y);

     if(W->client == c)
     {
          W->client = NULL;
          XSetInputFocus(W->dpy, W->root, RevertToPointerRoot, CurrentTime);
     }

     /* Remove from global client list */
     SLIST_REMOVE(&W->h.client, c, client, next);

     tag_client(NULL, c);

     ewmh_set_wm_state(c->win, WithdrawnState);

     XUngrabButton(W->dpy, AnyButton, AnyModifier, c->win);
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
