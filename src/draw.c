/*
*      draw.c
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

void
draw_text(Drawable d, int x, int y, char* fg, uint bg, int pad, char *str)
{
     XftColor xftcolor;
     XftDraw *xftd;

     /* Transform X Drawable -> Xft Drawable */
     xftd = XftDrawCreate(dpy, d, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));

     /* Color the text font */
     draw_rectangle(d, x - pad/2, 0, textw(str) + pad, barheight, bg);

     /* Alloc text color */
     XftColorAllocName(dpy, DefaultVisual(dpy, screen),
                       DefaultColormap(dpy, screen), fg, &xftcolor);

     /* Draw the text */
     XftDrawStringUtf8(xftd, &xftcolor, xftfont, x, y, (FcChar8 *)str, strlen(str));

     /* Free the text color and XftDraw */
     XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), &xftcolor);
     XftDrawDestroy(xftd);


     return;
}

void
draw_taglist(Drawable dr)
{
     int i;
     char buf[conf.ntag][256];
     char p[4];
     taglen[0] = PAD/2;

     for(i = 0; i < conf.ntag; ++i)
     {
          /* Make the tags string */
          ITOA(p, client_pertag(i+1));
          sprintf(buf[i], "%s<%s>", tags[i+1].name, (client_pertag(i+1)) ? p : "");

          /* Draw the string */
          draw_text(dr, taglen[i], fonth,
                 ((i+1 == seltag) ? conf.colors.tagselfg : conf.colors.text),
                 ((i+1 == seltag) ? conf.colors.tagselbg : conf.colors.bar), PAD, buf[i]);

          /* Draw the tag separation */
          draw_rectangle(dr, taglen[i] + textw(buf[i]) + PAD/2,
                         0, conf.tagbordwidth, barheight, conf.colors.tagbord);

          /* Edit taglen[i+1] for the next time */
          taglen[i+1] = taglen[i] + textw(buf[i]) + PAD + conf.tagbordwidth;
     }

     return;
}

void
draw_layout(void)
{
     /* Draw layout name / symbol */
     draw_text(bar->dr, taglen[conf.ntag], fonth,
               conf.colors.layout_fg,
               conf.colors.layout_bg,
               2, tags[seltag].layout.symbol);

     /* Draw right separation */
     draw_rectangle(bar->dr,
                    taglen[conf.ntag] + textw(tags[seltag].layout.symbol) + PAD/2,
                    0, conf.tagbordwidth, barheight, conf.colors.tagbord);

     return;
}

void
draw_rectangle(Drawable dr, int x, int y, uint w, uint h, uint color)
{
     XRectangle r = { x, y, w, h };

     XSetForeground(dpy, gc, color);
     XFillRectangles(dpy, dr, gc, &r, 1);

     return;
}

ushort
textw(const char *text)
{
     XGlyphInfo gl;

     XftTextExtentsUtf8(dpy, xftfont, (FcChar8 *)text, strlen(text), &gl);

     return gl.width + xftfont->descent;
}
