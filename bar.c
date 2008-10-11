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
bar_create(int x, int y, uint w, uint h, int bord, uint color)
{
     XSetWindowAttributes at;
     BarWindow *bw;

     bw = emalloc(1, sizeof(BarWindow));

     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     at.event_mask = ButtonPressMask | ExposureMask;

     bw->dr = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
     bw->win = XCreateWindow(dpy, root, x, y, w, h, bord, DefaultDepth(dpy, screen),
                             CopyFromParent, DefaultVisual(dpy, screen),
                             CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);

     bw->x = x; bw->y = y;
     bw->w = w; bw->h = h;
     bw->bord = bord;
     bw->color = color;

     return bw;
}

void
bar_delete(BarWindow *bw)
{
     XDestroyWindow(dpy, bw->win);
     XFreePixmap(dpy, bw->dr);

     return;
}

void
bar_moveresize(BarWindow *bw, int x, int y, uint w, uint h)
{
     if(w != bw->w || h != bw->h)
     {
          XFreePixmap(dpy, bw->dr);
          bw->dr = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
          bw->w = w; bw->h = h;
     }
     bw->x = x; bw->y = y;
     XMoveResizeWindow(dpy, bw->win, x, y, w, h);

     return;
}

void
bar_refresh_color(BarWindow *bw)
{
     XSetForeground(dpy, gc, bw->color);
     XFillRectangle(dpy, bw->dr, gc, 0, 0, bw->w, bw->h);

     return;
}

void
bar_refresh(BarWindow *bw)
{
     XCopyArea(dpy, bw->dr, bw->win, gc, 0, 0, bw->w, bw->h, 0, 0);

     return;
}

/* Top/Bottom Bar Manage Function */
void
updatebar(void)
{
     int  i , k, sp = 6;
     char buf[conf.ntag][256];
     char p[4];

     bar_refresh_color(bar);

     /* TAGLIST */
     {
          for(i = 0; i < conf.ntag; ++i)
          {
               /* Make the tags string */
               ITOA(p, clientpertag(i+1));
               sprintf(buf[i], "%s<%s>", tags[i+1].name, (clientpertag(i+1)) ? p : "");
               taglen[i+1] = taglen[i] + textw(buf[i]) + sp;

               /* Draw tags */
               xprint(bar->dr, taglen[i], fonth,
                      ((i+1 == seltag) ? conf.colors.tagselfg : conf.colors.text),
                      ((i+1 == seltag) ? conf.colors.tagselbg : conf.colors.bar), sp-1, -sp, buf[i]);

               /* Tags border separation */
               XSetForeground(dpy, gc, conf.colors.tagbord);
               if(i > 0)
                    XFillRectangle(dpy, bar->dr, gc, taglen[i]-sp+1, 0, conf.tagbordwidth, barheight);
          }
          XFillRectangle(dpy, bar->dr, gc, taglen[conf.ntag]-sp+1, 0, conf.tagbordwidth, barheight);
     }

     /* Layout symbol */
     {
          xprint(bar->dr, taglen[conf.ntag] - sp/3, fonth,
                 conf.colors.layout_fg, conf.colors.layout_bg,
                 0, 0, tags[seltag].layout.symbol);
     }

     /* Draw status text */
     k = textw(bartext);
     xprint(bar->dr, mw-k, fonth, conf.colors.text, conf.colors.bar, 0, 0, bartext);

     /* Bar border */
     if(conf.tagbordwidth)
     {
          XSetForeground(dpy, gc, conf.colors.tagbord);
          XFillRectangle(dpy, bar->dr, gc, 0,
                         ((conf.bartop) ? barheight-1: 0), mw, 1);
          XFillRectangle(dpy, bar->dr, gc, mw-k-5, 0, 1, barheight);
     }

     bar_refresh(bar);
     XSync(dpy, False);

     /* Update Bar Buttons */
     updatebutton(True);

     return;
}

/* BARBUTTON MANAGE FUNCTION
 * if c is False, you can execute this function for the first time
 * else the button is just updated *TestingButWorking* */
void
updatebutton(Bool c)
{
     int i, j, x, pm = 0;
     int y = 0, hi = 0;
     XSetWindowAttributes at;

     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     at.event_mask = ButtonPressMask | ExposureMask;

     j = taglen[conf.ntag] + textw(tags[seltag].layout.symbol) + xftfont->ascent;

     if(!conf.bartop)
          y = bary + 1;

     if(conf.tagbordwidth)
          hi = -1;

     for(i = 0; i < conf.nbutton; ++i)
     {
          if(!(x = conf.barbutton[i].x))
          {
               if(i)
                    pm += textw(conf.barbutton[i-1].text);
               x = (!i) ? j : j + pm;
          }

          if(!c)
          {
               conf.barbutton[i].bw = bar_create(x, y,
                                                  textw(conf.barbutton[i].text),
                                                  barheight + hi,
                                                  0, conf.barbutton[i].bg_color);
               XMapRaised(dpy, conf.barbutton[i].bw->win);
          }

          if(!conf.barbutton[i].bw)
               return;

          bar_refresh_color(conf.barbutton[i].bw);
          bar_moveresize(conf.barbutton[i].bw, x, y, textw(conf.barbutton[i].text), barheight + hi);
          xprint(conf.barbutton[i].bw->dr, 1, fonth, conf.barbutton[i].fg_color,
                 conf.barbutton[i].bg_color, 0, 0, conf.barbutton[i].text);
          bar_refresh(conf.barbutton[i].bw);
     }
     XSync(dpy, False);

     return;
}
