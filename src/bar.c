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

BarWindow*
bar_create(int x, int y, uint w, uint h, int bord, uint color, Bool entermask)
{
     XSetWindowAttributes at;
     BarWindow *bw;

     bw = emalloc(1, sizeof(BarWindow));

     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     if(entermask)
          at.event_mask = ButtonPressMask | ExposureMask | EnterWindowMask;
     else
          at.event_mask = ButtonPressMask | ExposureMask;

     bw->dr = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
     bw->win = XCreateWindow(dpy, root, x, y, w, h, bord, DefaultDepth(dpy, screen),
                             CopyFromParent, DefaultVisual(dpy, screen),
                             CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);

     bw->geo.x = x; bw->geo.y = y;
     bw->geo.width = w; bw->geo.height = h;
     bw->bord = bord;
     bw->color = color;

     return bw;
 }

void
bar_delete(BarWindow *bw)
{
     XDestroyWindow(dpy, bw->win);
     XFreePixmap(dpy, bw->dr);
     free(bw);

     return;
}

void
bar_map(BarWindow *bw)
{
     if(!bw->mapped)
     {
          XMapRaised(dpy, bw->win);
          bw->mapped = True;
     }

     return;
}

void
bar_unmap(BarWindow *bw)
{
     if(bw->mapped)
     {
          XUnmapWindow(dpy, bw->win);
          bw->mapped = False;
     }

     return;
}

void
bar_move(BarWindow *bw, int x, int y)
{
     bw->geo.x = x;
     bw->geo.y = y;

     XMoveWindow(dpy, bw->win, x, y);

     return;
}

void
bar_resize(BarWindow *bw, uint w, uint h)
{
     bw->geo.width = w;
     bw->geo.height = h;
     XFreePixmap(dpy, bw->dr);
     bw->dr = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));

     XResizeWindow(dpy, bw->win, w, h);

     return;
}

void
bar_refresh_color(BarWindow *bw)
{
     draw_rectangle(bw->dr, 0, 0, bw->geo.width, bw->geo.height, bw->color);
     if(bw->bord)
          XSetWindowBorder(dpy, bw->win, bw->color);

     return;
}

void
bar_refresh(BarWindow *bw)
{
     XCopyArea(dpy, bw->dr, bw->win, gc, 0, 0, bw->geo.width, bw->geo.height, 0, 0);

     return;
}

/* Top/Bottom Bar Manage Function */
void
updatebar(void)
{
     char buf[256];

     /* Refresh bar color */
     bar_refresh_color(infobar.bar);

     /* Draw taglist */
     draw_taglist(infobar.bar->dr);

     /* Draw layout symbol */
     draw_layout();

     /* Draw mwfact && nmaster info */
     sprintf(buf, "mwfact: %.2f - nmaster: %d",
             tags[seltag].mwfact,
             tags[seltag].nmaster);
     draw_text(infobar.bar->dr, infobar.lastsep + PAD/1.5, fonth, conf.colors.text, conf.colors.bar, 0, buf);
     draw_rectangle(infobar.bar->dr, textw(buf) + infobar.lastsep + PAD,
                    0, conf.tagbordwidth, infobar.geo.height, conf.colors.tagbord);

     /* Draw status text */
     draw_text(infobar.bar->dr, mw - textw(infobar.statustext), fonth, conf.colors.text, conf.colors.bar, 0, infobar.statustext);

     /* Bar border */
     if(conf.tagbordwidth)
     {
          draw_rectangle(infobar.bar->dr, 0, ((conf.bartop) ? infobar.geo.height - 1: 0),
                         mw, 1, conf.colors.tagbord);
          draw_rectangle(infobar.bar->dr, mw - textw(infobar.statustext) - 5,
                         0, conf.tagbordwidth, infobar.geo.height, conf.colors.tagbord);
     }

     /* Refresh the bar */
     bar_refresh(infobar.bar);

     return;
}

void
uicb_togglebarpos(uicb_t cmd)
{
     conf.bartop = !conf.bartop;
     if(conf.bartop)
          sgeo.y = conf.titlebar.pos ? infobar.geo.height : infobar.geo.height + conf.titlebar.height;
     else
          sgeo.y = conf.titlebar.pos ? 0 : conf.titlebar.height;
     infobar.geo.y = (conf.bartop) ? 0 : mh - infobar.geo.height;
     bar_move(infobar.bar, 0, infobar.geo.y);
     updatebar();
     arrange();

     return;
}
