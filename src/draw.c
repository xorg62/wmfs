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
static int sw = 0;

/** Draw an image in a drawable
  * \param dr Drawable
  * \param x X position
  * \param y Y position
  * \param name Path of the image
*/
static void
draw_image(Drawable dr, int x, int y, int w, int h, char *name)
{
     Imlib_Image image;

     if(!dr)
          return;

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
          imlib_render_image_on_drawable_at_size(x, y, w, h);
     else
          warnx("Can't draw image: '%s'", name);

     imlib_free_image();

     return;
}

/** Check images blocks in str and return properties
  * --> \i[x;y;w;h;name]\
  *\param im ImageAttr pointer, image properties
  *\param str String
  *\param m Check dynamic mouse
  *\return n Lenght of i
  */
static void
parse_image_block(Drawable dr, char *str, bool m)
{
     ImageAttr im;
     BarWindow *bw;
     Geo area;
     char as, mouse[512] = { 0 };
     int i = 0, j = 0, k = 0, n;

     for(; i < (int)strlen(str); ++i, ++j)
          if((n = sscanf(&str[i], "\\i[%d;%d;%d;%d;%512[^];]]%c",
                              &im.x, &im.y, &im.w, &im.h, im.name, &as)) == 6
                    || (n = sscanf(&str[i], "\\i[%d;%d;%d;%d;%512[^;];%512[^]]]%c",
                              &im.x, &im.y, &im.w, &im.h, im.name, mouse, &as)) == 7
                    && as == '\\')
          {
               draw_image(dr, im.x - sw, im.y, im.w, im.h, im.name);

               /* Etablish clickable area on image (on infobar wins only (status mouse bind) */
               if(n == 7 && m)
               {
                    area.x = im.x - sw;
                    area.y = im.y;
                    area.width  = im.w;
                    area.height = im.h;

                    /* Associate drawable with window; travel infobars */
                    for(; k < screen_count(); ++k)
                         if(infobar[k].bar->dr == dr)
                         {
                              statustext_mouse(mouse, area, &infobar[k]);
                              break;
                         }
               }

               for(++i, --j; str[i] != as || str[i - 1] != ']'; ++i);
          }
          else if(j != i)
               str[j] = str[i];

     for(sw = 0, k = j; k < i; str[k++] = 0);

     return;
}
#endif /* HAVE_IMLIB */

/** Draw a string in a Drawable
 * \param d Drawable
 * \param x X position
 * \param y Y position
 * \param fg Foreground text color
 * \param pad Text padding
 * \param str String that will be draw
*/
void
draw_text(Drawable d, int x, int y, char* fg, char *str)
{
     CHECK(str);

     /* To draw image everywhere we can draw text */
#ifdef HAVE_IMLIB
     char *ostr;
     size_t textlen = 0;

     if(strstr(str, "i["))
     {
          if(d == infobar[conf.systray.screen].bar->dr)
               sw = systray_get_width();

          ostr = xstrdup(str);
          textlen = strlen(ostr);
          parse_image_block(d, str, True);
     }
#endif /* HAVE_IMLIB */

#ifdef HAVE_XFT
     if(conf.use_xft)
     {
          XftColor xftcolor;
          XftDraw *xftd;

          /* Transform X Drawable -> Xft Drawable */
          xftd = XftDrawCreate(dpy, d, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN));

          /* Alloc text color */
          XftColorAllocName(dpy, DefaultVisual(dpy, SCREEN),
                    DefaultColormap(dpy, SCREEN), fg, &xftcolor);

          XftDrawStringUtf8(xftd, &xftcolor, font.font, x, y, (FcChar8 *)str, strlen(str));

          /* Free the text color and XftDraw */
          XftColorFree(dpy, DefaultVisual(dpy, SCREEN), DefaultColormap(dpy, SCREEN), &xftcolor);

          XftDrawDestroy(xftd);
     }
     else
#endif /* HAVE_XFT */
     {
          /* Use font set */
          XSetForeground(dpy, gc, getcolor(fg));
          XmbDrawString(dpy, d, font.fontset, gc, x, y, str, strlen(str));
     }


#ifdef HAVE_IMLIB
     if(textlen)
     {
          strncpy(str, ostr, textlen);
          free(ostr);
     }
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
draw_rectangle(Drawable dr, int x, int y, int w, int h, uint color)
{
     XSetForeground(dpy, gc, color);
     XFillRectangle(dpy, dr, gc, x, y, (uint)w, (uint)h);

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
draw_graph(Drawable dr, int x, int y, int w, int h, uint color, char *data)
{
     int i;

     for(i = 0; i < w; ++i)
          draw_rectangle(dr, x + i, y + h - data[i], 1, data[i], color);

     return;
}

/** Calculates the text's size relatively to the font
 * \param text Text string
 * \return final text width
*/
ushort
textw(char *text)
{
     Drawable d = 0;
     ushort ret = 0;

     if(!text)
          return 0;

#ifdef HAVE_IMLIB
     char *ostr;
     size_t textlen = 0;

     if(strstr(text, "i["))
     {
          ostr = xstrdup(text);
          textlen = strlen(ostr);
          parse_image_block(d, text, False);
     }
#endif /* HAVE_IMLIB */

#ifdef HAVE_XFT
     if(conf.use_xft)
     {
          XGlyphInfo gl;

          XftTextExtentsUtf8(dpy, font.font, (FcChar8 *)text, strlen(text), &gl);
          ret = gl.width + font.de;
     }
     else
#endif /* HAVE_XFT */
     {
          XRectangle r;

          XmbTextExtents(font.fontset, text, strlen(text), NULL, &r);
          ret = r.width;
     }

#ifdef HAVE_IMLIB
     if(textlen)
     {
          strncpy(text, ostr, textlen);
          free(ostr);
     }
#endif /* HAVE_IMLIB */

     return ret;
}
