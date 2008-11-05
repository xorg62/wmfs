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
     draw_rectangle(d, x - pad/2, 0, textw(str) + pad, infobar.geo.height, bg);

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
                         0, conf.tagbordwidth, infobar.geo.height, conf.colors.tagbord);

          /* Edit taglen[i+1] for the next time */
          taglen[i+1] = taglen[i] + textw(buf[i]) + PAD + conf.tagbordwidth;
     }

     return;
}

void
draw_layout(void)
{
     int px, py, width;
     char symbol[256];

     /* Set symbol & position */
     px = width = taglen[conf.ntag];
     py = conf.bartop ? infobar.geo.y : infobar.geo.y + 1;
     if(tags[seltag].layout.func == freelayout
        || tags[seltag].layout.func == maxlayout)
          strcpy(symbol, tags[seltag].layout.symbol);
     else
          strcpy(symbol, conf.tile_symbol);

     /* Draw layout name/symbol */
     bar_refresh_color(infobar.layout_switch);

     bar_move(infobar.layout_switch, px, py);
     bar_resize(infobar.layout_switch, textw(symbol) + PAD, infobar.geo.height - 1);
     draw_text(infobar.layout_switch->dr, PAD/2, fonth,
               conf.colors.layout_fg,
               conf.colors.layout_bg,
               PAD, symbol);
     width += textw(symbol) + PAD;
     bar_refresh(infobar.layout_switch);

     if(tags[seltag].layout.func == tile
        || tags[seltag].layout.func == tile_left
        || tags[seltag].layout.func == tile_top
        || tags[seltag].layout.func == tile_bottom
        || tags[seltag].layout.func == grid)
     {
          bar_map(infobar.layout_type_switch);
          bar_refresh_color(infobar.layout_type_switch);
          bar_move(infobar.layout_type_switch, px + infobar.layout_switch->geo.width + PAD/2, py);
          bar_resize(infobar.layout_type_switch, textw(tags[seltag].layout.symbol) + PAD, infobar.geo.height - 1);
          draw_text(infobar.layout_type_switch->dr, PAD/2, fonth,
                    conf.colors.layout_fg,
                    conf.colors.layout_bg,
                    PAD, tags[seltag].layout.symbol);
          width += textw(tags[seltag].layout.symbol) + PAD * 1.5;

          bar_refresh(infobar.layout_type_switch);
     }
     else
          bar_unmap(infobar.layout_type_switch);

     /* Draw right separation */
     infobar.lastsep = width + PAD / 2;
     draw_rectangle(infobar.bar->dr, infobar.lastsep, 0, conf.tagbordwidth, infobar.geo.height, conf.colors.tagbord);

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

     if(!text)
          return 0;

     XftTextExtentsUtf8(dpy, xftfont, (FcChar8 *)text, strlen(text), &gl);

     return gl.width + xftfont->descent;
}
