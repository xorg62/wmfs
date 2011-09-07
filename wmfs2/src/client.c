/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "client.h"
#include "config.h"
#include "util.h"
#include "barwin.h"

#define CLIENT_MOUSE_MOD Mod1Mask

/** Send a ConfigureRequest event to the struct Client
 * \param c struct Client pointer
*/
void
client_configure(struct Client *c)
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

struct Client*
client_gb_win(Window w)
{
     struct Client *c = SLIST_FIRST(&W->h.client);

     while(c && c->win != w)
          c = SLIST_NEXT(c, next);

     return c;
}

/** Map a client
 * \param c struct Client pointer
 */
void
client_map(struct Client *c)
{
     XMapWindow(W->dpy, c->win);
}

/** Unmap a client
 * \param c struct Client pointer
 */
void
client_unmap(struct Client *c)
{
     XUnmapWindow(W->dpy, c->win);
}

static void
client_grabbuttons(struct Client *c, bool focused)
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
client_focus(struct Client *c)
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
     else
          XSetInputFocus(W->dpy, W->root, RevertToPointerRoot, CurrentTime);

}

/** Get a client name
 * \param c struct Client pointer
*/
void
client_get_name(struct Client *c)
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
 * \param c struct Client pointer
*/
void
client_close(struct Client *c)
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

struct Client*
client_new(Window w, XWindowAttributes *wa)
{
     struct Client *c;

     c = xcalloc(1, sizeof(struct Client));

     /* C attributes */
     c->win    = w;
     c->screen = W->screen;
     c->flags  = 0;

     /* Set tag */
     tag_client(W->screen->seltag, c);

     /* struct Geometry */
     c->geo.x = wa->x;
     c->geo.y = wa->y;
     c->geo.w = wa->width;
     c->geo.h = wa->height;

     /* X window attributes */
     XSelectInput(W->dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

     XSetWindowBorder(W->dpy, w, THEME_DEFAULT->client_n.bg);
     XSetWindowBorderWidth(W->dpy, w, THEME_DEFAULT->client_border_width);
     client_grabbuttons(c, false);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.client, c, next);

     XMoveResizeWindow(W->dpy, w,
               c->screen->ugeo.x,
               c->screen->ugeo.y,
               c->screen->ugeo.w,
               c->screen->ugeo.h);


     client_map(c);

     XRaiseWindow(W->dpy, w);

     client_configure(c);

     return c;
}

void
client_remove(struct Client *c)
{
     struct Client *cc;

     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);

     if(W->client == c)
          client_focus(NULL);

     if(c->tag->sel == c)
          c->tag->sel = SLIST_FIRST(&c->tag->clients);

     SLIST_REMOVE(&W->h.client, c, Client, next);

     tag_client(NULL, c);

     XSync(W->dpy, False);
     XUngrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler);

     free(c);
}

void
client_free(void)
{
     FREE_LIST(Client, W->h.client);
}
