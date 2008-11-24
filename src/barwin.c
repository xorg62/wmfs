/*
*      bar.c
*      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

/** Create a BarWindow
 * \param parent Parent window of the BarWindow
 * \param x X position
 * \param y Y position
 * \param w BarWindow Width
 * \param h BarWindow Height
 * \param color BarWindow color
 * \param entermask Bool for know if the EnterMask mask is needed
 * \return The BarWindow pointer
*/
BarWindow*
barwin_create(Window parent,
           int x, int y, uint w, uint h,
           uint color, Bool entermask)
{
     XSetWindowAttributes at;
     BarWindow *bw;

     bw = emalloc(1, sizeof(BarWindow));

     at.override_redirect = True;
     at.background_pixmap = ParentRelative;
     if(entermask)
          at.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
               ButtonPressMask | ExposureMask | EnterWindowMask |
               LeaveWindowMask | StructureNotifyMask;
     else
          at.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
               ButtonPressMask | ExposureMask | StructureNotifyMask;

     /* Create window */
     bw->win = XCreateWindow(dpy, parent, x, y, w, h, 0, DefaultDepth(dpy, screen),
                             CopyFromParent, DefaultVisual(dpy, screen),
                             CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);
     bw->dr = XCreatePixmap(dpy, parent, w, h, DefaultDepth(dpy, screen));

     /* His border */
     CWIN(bw->border.left,   bw->win, 0, 0, SHADH, h, 0, CWBackPixel, color_enlight(color), &at);
     CWIN(bw->border.top,    bw->win, 0, 0, w, SHADH, 0, CWBackPixel, color_enlight(color), &at);
     CWIN(bw->border.bottom, bw->win, 0, h - SHADH, w, SHADH, 0, CWBackPixel, SHADC, &at);
     CWIN(bw->border.right,  bw->win, w - SHADH, 0, SHADH, h, 0, CWBackPixel, SHADC, &at);

     bw->geo.x = x;
     bw->geo.y = y;
     bw->geo.width = w;
     bw->geo.height = h;
     bw->color = color;
     bw->border.light = color_enlight(color);
     bw->border.dark = SHADC;

     return bw;
}

/** Delete a BarWindow
 * \param bw BarWindow pointer
*/
void
barwin_delete(BarWindow *bw)
{
     CHECK(bw);

     XSelectInput(dpy, bw->win, NoEventMask);
     XDestroyWindow(dpy, bw->win);
     XFreePixmap(dpy, bw->dr);
     free(bw);

     return;
}

/** Delete the BarWindow sub windows
 * \param bw BarWindow pointer
*/
void
barwin_delete_subwin(BarWindow *bw)
{
     CHECK(bw);

     XDestroySubwindows(dpy, bw->win);

     return;
}

/** Map a BarWindow
 * \param bw BarWindow pointer
*/
void
barwin_map(BarWindow *bw)
{
     CHECK(!bw->mapped);

     XMapWindow(dpy, bw->win);

     bw->mapped = True;

     return;
}


/** Map the subwindows of a BarWindow
 *  Use for the BarWindow special border...
 * \param bw BarWindow pointer
 */
void
barwin_map_subwin(BarWindow *bw)
{
     CHECK(bw);

     XMapSubwindows(dpy, bw->win);

     return;
}

/** Unmap a BarWindow
 * \param bw BarWindow pointer
*/
void
barwin_unmap(BarWindow *bw)
{
     CHECK(bw->mapped);

     XUnmapWindow(dpy, bw->win);

     bw->mapped = False;

     return;
}

/** Unmap the BarWindow sub windows
 * \param bw BarWindow pointer
*/
void
barwin_unmap_subwin(BarWindow *bw)
{
     CHECK(bw);

     XUnmapSubwindows(dpy, bw->win);

     return;
}

/** Move a BarWindow
 * \param bw BarWindow pointer
 * \param x X position
 * \param y Y poistion
*/
void
barwin_move(BarWindow *bw, int x, int y)
{
     CHECK(bw);

     bw->geo.x = x;
     bw->geo.y = y;

     XMoveWindow(dpy, bw->win, x, y);

     return;
}

/** Resize a BarWindow
 * \param bw BarWindow pointer
 * \param w Width
 * \param h Height
*/
void
barwin_resize(BarWindow *bw, uint w, uint h)
{
     CHECK(bw);

     bw->geo.width = w;
     bw->geo.height = h;
     XFreePixmap(dpy, bw->dr);

     /* Frame */
     bw->dr = XCreatePixmap(dpy, root, w - SHADH, h - SHADH, DefaultDepth(dpy, screen));
     XResizeWindow(dpy, bw->win, w, h);

     /* Border */
     XResizeWindow(dpy, bw->border.left, SHADH, h);
     XResizeWindow(dpy, bw->border.top, w, SHADH);
     XResizeWindow(dpy, bw->border.bottom, w, SHADH);
     XMoveResizeWindow(dpy, bw->border.right, w - SHADH, 0, SHADH, h);


     return;
}

/** Refresh the BarWindow Color
 * \param bw BarWindow pointer
*/
void
barwin_refresh_color(BarWindow *bw)
{
     CHECK(bw);

     draw_rectangle(bw->dr, 0, 0, bw->geo.width, bw->geo.height, bw->color);

     XSetWindowBackground(dpy, bw->border.left ,   bw->border.light);
     XSetWindowBackground(dpy, bw->border.top ,    bw->border.light);
     XSetWindowBackground(dpy, bw->border.bottom , bw->border.dark);
     XSetWindowBackground(dpy, bw->border.right ,  bw->border.dark);

     XClearWindow(dpy, bw->border.left);
     XClearWindow(dpy, bw->border.top);
     XClearWindow(dpy, bw->border.bottom);
     XClearWindow(dpy, bw->border.right);

     return;
}

/** Refresh the BarWindow
 * \param bw BarWindow pointer
*/
void
barwin_refresh(BarWindow *bw)
{
     CHECK(bw);

     XCopyArea(dpy, bw->dr, bw->win, gc, 0, 0, bw->geo.width, bw->geo.height, 0, 0);

     return;
}
