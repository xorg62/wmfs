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

     XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ev);
}

Client*
client_gb_win(Window *w)
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
     XSetWindowAttributes at;

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
     at.event_mask = PropertyChangeMask;
     XChangeWindowAttributes(dpy, w, CWEventMask, &at);

     /* Attach */
     SLIST_INSERT(&W->h.client, c, Client, next);

     return c;
}

void
client_remove(Client *c)
{
     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);

     XReparentWindow(W->dpy, c->win, W->root, c->geo.x, c->geo.y);

     SLIST_REMOVE(&W->h.client, c, Client, next);

     XUngrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler);

     free(c);
}

void
client_free(void)
{
     FREE_LIST(W->client);
}
