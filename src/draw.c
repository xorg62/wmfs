/*
*      draw.c
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
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

/** Draw a string in a Drawable
 * \param d Drawable
 * \param x X position
 * \param y Y position
 * \param fg Foreground text color
 * \param pad Text padding
 * \param str String that will be draw
*/
void
draw_text(Drawable d, int x, int y, char* fg, int pad, char *str)
{
     XftColor xftcolor;
     XftDraw *xftd;

     if(!str)
          return;

     /* Transform X Drawable -> Xft Drawable */
     xftd = XftDrawCreate(dpy, d, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN));

     /* Alloc text color */
     XftColorAllocName(dpy, DefaultVisual(dpy, SCREEN),
                       DefaultColormap(dpy, SCREEN), fg, &xftcolor);

     XftDrawStringUtf8(xftd, &xftcolor, font, x, y, (FcChar8 *)str, strlen(str));

     /* Free the text color and XftDraw */
     XftColorFree(dpy, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN), &xftcolor);

     XftDrawDestroy(xftd);

     return;
}

/** Draw a Rectangle in a drawable
 * \param dr Drawable
 * \param x X position
 * \param y Y position
 * \param w Width
 * \param h Height
 * \param color Color of the rectangle
*/
void
draw_rectangle(Drawable dr, int x, int y, uint w, uint h, uint color)
{
     XRectangle r = { x, y, w, h };

     XSetForeground(dpy, gc, color);
     XFillRectangles(dpy, dr, gc, &r, 1);

     return;
}

/** Calculates the text's size relatively to the font
 * \param text Text string
 * \return final text width
*/
ushort
textw(const char *text)
{
     XGlyphInfo gl;

     if(!text)
          return 0;

     XftTextExtentsUtf8(dpy, font, (FcChar8 *)text, strlen(text), &gl);

     return gl.width + font->descent;
}


