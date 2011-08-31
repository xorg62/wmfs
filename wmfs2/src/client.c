/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "client.h"
#include "util.h"

/** Send a ConfigureRequest event to the Client
 * \param c Client pointer
*/
void
client_configure(Client *c)
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

Client*
client_gb_win(Window w)
{
     Client *c = SLIST_FIRST(&W->h.client);

     while(c && c->win != w)
          c = SLIST_NEXT(c, next);

     return c;
}

/** Map a client
 * \param c Client pointer
 */
void
client_map(Client *c)
{
     XMapWindow(W->dpy, c->win);
}

/** Unmap a client
 * \param c Client pointer
 */
void
client_unmap(Client *c)
{
     XUnmapWindow(W->dpy, c->win);
}

void
client_focus(Client *c)
{
     /* Unfocus selected */
     if(c->tag->sel && c->tag->sel != c)
     {
          XSetWindowBorder(W->dpy, c->win, 0xf0f0f0);
     }

     /* Focus c */
     if((c->tag->sel = c))
     {
          XSetWindowBorder(W->dpy, c->win, 0xffffff);

          XSetInputFocus(W->dpy, c->win, RevertToPointerRoot, CurrentTime);

          XRaiseWindow(W->dpy, c->win);
     }
     /* Else, set input focus to root window */
     else
          XSetInputFocus(W->dpy, W->root, RevertToPointerRoot, CurrentTime);

}

/** Get a client name
 * \param c Client pointer
*/
void
client_get_name(Client *c)
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
 * \param c Client pointer
*/
void
client_close(Client *c)
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

Client*
client_new(Window w, XWindowAttributes *wa)
{
     Client *c;

     c = xcalloc(1, sizeof(Client));

     /* C attributes */
     c->win    = w;
     c->screen = W->screen;
     c->tag    = W->screen->seltag;
     c->flags  = 0;

     /* Geometry */
     c->geo.x = wa->x;
     c->geo.y = wa->y;
     c->geo.w = wa->width;
     c->geo.h = wa->height;

     /* X window attributes */
     XSelectInput(W->dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

     XSetWindowBorder(W->dpy, w, 0xffffff);
     XSetWindowBorderWidth(W->dpy, w, 1);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.client, c, next);

     client_map(c);

     XMoveResizeWindow(W->dpy, w, c->geo.x, c->geo.y, c->geo.w, c->geo.h);
     XRaiseWindow(W->dpy, w);

     client_focus(c);
     client_configure(c);

     return c;
}

void
client_remove(Client *c)
{
     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);

     SLIST_REMOVE(&W->h.client, c, Client, next);

     XUngrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler);

     free(c);
}

void
client_free(void)
{
     FREE_LIST(Client, W->h.client);
}
