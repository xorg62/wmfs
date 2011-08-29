/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "barwin.h"
#include "util.h"

/** Create a Barwin
 * \param parent Parent window of the BarWindow
 * \param x X position
 * \param y Y position
 * \param w Barwin Width
 * \param h Barwin Height
 * \param color Barwin color
 * \param entermask bool for know if the EnterMask mask is needed
 * \return The BarWindow pointer
*/
Barwin*
barwin_new(Window parent, int x, int y, int w, int h, Color fg, Color bg, bool entermask)
{
     Barwin *b = (Barwin*)xcalloc(1, sizeof(Barwin));
     XSetWindowAttributes at =
     {
          .override_redirect = True,
          .background_pixmap = ParentRelative,
          .event_mask = BARWIN_MASK
     };

     if(entermask)
          at.event_mask |= BARWIN_ENTERMASK;

     /* Create window */
     b->win = XCreateWindow(W->dpy, parent, x, y, w, h, 0, W->xdepth, CopyFromParent,
               DefaultVisual(W->dpy, W->xscreen), BARWIN_WINCW, &at);

     b->dr = XCreatePixmap(W->dpy, parent, w, h, W->xdepth);

     /* Property */
     b->geo.x = x;
     b->geo.y = y;
     b->geo.w = w;
     b->geo.h = h;
     b->bg = bg;
     b->fg = fg;

     SLIST_INIT(&b->mousebinds);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.barwin, b, next);

     return b;
}

/** Delete a Barwin
 * \param bw Barwin pointer
*/
void
barwin_remove(Barwin *b)
{
     SLIST_REMOVE(&W->h.barwin, b, Barwin, next);

     XSelectInput(W->dpy, b->win, NoEventMask);
     XDestroyWindow(W->dpy, b->win);
     XFreePixmap(W->dpy, b->dr);

     /* Free mousebinds */
     FREE_LIST(Mousebind, b->mousebinds);

     free(b);
}

/** Resize a Barwin
 * \param bw Barwin pointer
 * \param w Width
 * \param h Height
*/
void
barwin_resize(Barwin *b, int w, int h)
{
     /* Frame */
     XFreePixmap(W->dpy, b->dr);

     b->dr = XCreatePixmap(W->dpy, W->root, w, h, W->xdepth);

     b->geo.w = w;
     b->geo.h = h;

     XResizeWindow(W->dpy, b->win, w, h);
}

void
barwin_mousebind_new(Barwin *b, unsigned int button, bool u, Geo a, void (*func)(Uicb), Uicb cmd)
{
     Mousebind *m;

     m = xcalloc(1, sizeof(Mousebind));

     m->button = button;
     m->use_area = u;
     m->area = a;
     m->func = func;

     m->cmd = (cmd ? xstrdup(cmd) : NULL);

     SLIST_INSERT_HEAD(&b->mousebinds, m, next);
}

/** Refresh the Barwin Color
 * \param bw Barwin pointer
*/
void
barwin_refresh_color(Barwin *b)
{
     XSetForeground(W->dpy, W->gc, b->bg);
     XFillRectangle(W->dpy, b->dr, W->gc, 0, 0, b->geo.w, b->geo.h);
}



