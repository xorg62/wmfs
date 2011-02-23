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

#ifdef HAVE_IMLIB
/** Draw an image in a drawable
  * \param dr Drawable
  * \param x X position
  * \param y Y position
  * \param name Path of the image
*/
static void draw_image(Drawable dr, int x, int y, int w, int h, char *name)
{
     Imlib_Image image;

     if(!name)
          return;

     imlib_set_cache_size(2048 * 1024);
     imlib_context_set_display(dpy);
     imlib_context_set_visual(DefaultVisual(dpy, DefaultScreen(dpy)));
     imlib_context_set_colormap(DefaultColormap(dpy, DefaultScreen(dpy)));
     imlib_context_set_drawable(dr);

     image = imlib_load_image(patht(name));
     imlib_context_set_image(image);

     if(w <= 0)
          w = imlib_image_get_width();

     if(h <= 0)
          h = imlib_image_get_height();

     if(image)
     {
          imlib_render_image_on_drawable_at_size(x, y, w, h);
          imlib_free_image();
     }
     else
          warnx("Can't draw image: '%s'", name);

     return;
}
#endif /* HAVE_IMLIB */

void
draw_text(Drawable d, int x, int y, char* fg, char *str)
{
     draw_image_ofset_text(d, x, y, fg, str, 0, 0);
}

/** Draw a string in a Drawable
 * \param d Drawable
 * \param x X position
 * \param y Y position
 * \param fg Foreground text color
 * \param pad Text padding
 * \param str String that will be draw
*/
void
draw_image_ofset_text(Drawable d, int x, int y, char* fg, char *str, int x_image_ofset, int y_image_ofset)
{
     XftColor xftcolor;
     XftDraw *xftd;
#ifdef HAVE_IMLIB
     char *ostr = NULL;
     int i, ni, sw = 0;
     ImageAttr im[128];
     size_t textlen;
#else
     (void)x_image_ofset;
     (void)y_image_ofset;
#endif /* HAVE_IMLIB */

     if(!str)
          return;

     /* To draw image everywhere we can draw text */
#ifdef HAVE_IMLIB

     ostr = xstrdup(str);
     textlen = strlen(ostr);

     if(strstr(str, "i["))
     {
          ni = parse_image_block(im, str);

          if(infobar[conf.systray.screen].bar && d == infobar[conf.systray.screen].bar->dr)
               sw = systray_get_width();

          for(i = 0; i < ni; ++i)
               draw_image(d, x_image_ofset + im[i].x - sw, y_image_ofset + im[i].y, im[i].w, im[i].h, im[i].name);
     }
#endif /* HAVE_IMLIB */

     /* Transform X Drawable -> Xft Drawable */
     xftd = XftDrawCreate(dpy, d, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN));

     /* Alloc text color */
     XftColorAllocName(dpy, DefaultVisual(dpy, SCREEN),
                       DefaultColormap(dpy, SCREEN), fg, &xftcolor);

     XftDrawStringUtf8(xftd, &xftcolor, font, x, y, (FcChar8 *)str, strlen(str));

     /* Free the text color and XftDraw */
     XftColorFree(dpy, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN), &xftcolor);

     XftDrawDestroy(xftd);

#ifdef HAVE_IMLIB
     if(strstr(ostr, "i["))
          strncpy(str, ostr, textlen);

     free(ostr);
#endif /* HAVE_IMLIB */

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

/** Draw a Graph in a drawable
 * \param dr Drawable
 * \param x X position
 * \param y Y position
 * \param w Width
 * \param h Height
 * \param color Color of the graph
 * \param data Array of bytes that will be draw
*/
void
draw_graph(Drawable dr, int x, int y, uint w, uint h, uint color, char *data)
{
     uint i;

     XSetForeground(dpy, gc, color);

     for(i = 0; i < w; ++i)
     {
          XRectangle r = { (x + i), (y + h - data[i]), 1, data[i] };

          XFillRectangles(dpy, dr, gc, &r, 1);
     }

     return;
}

/** Calculates the text's size relatively to the font
 * \param text Text string
 * \return final text width
*/
ushort
textw(char *text)
{
     XGlyphInfo gl;
#ifdef HAVE_IMLIB
     char *ostr = NULL;
     ImageAttr im[128];
     size_t textlen;
#endif /* HAVE_IMLIB */

     if(!text)
          return 0;


#ifdef HAVE_IMLIB

     ostr = xstrdup(text);
     textlen = strlen(ostr);

     if(strstr(text, "i["))
          parse_image_block(im, text);
#endif /* HAVE_IMLIB */

     XftTextExtentsUtf8(dpy, font, (FcChar8 *)text, strlen(text), &gl);

#ifdef HAVE_IMLIB
     if(strstr(ostr, "i["))
          strncpy(text, ostr, textlen);

     free(ostr);
#endif /* HAVE_IMLIB */

     return gl.width + font->descent;
}
