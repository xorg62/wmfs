/*
*      screen.c
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

/** Count the screens
 *\return the number of screen
*/
int
screen_count(void)
{
     int n = 0;

     if(XineramaIsActive(dpy))
          XineramaQueryScreens(dpy, &n);
     else
          n = ScreenCount(dpy);

     return n;
}

/** Get screen geometry by number
 *\param s Screen number
 *\return XRectangle struct
*/
XRectangle
screen_get_geo(int s)
{
     int n = 0;
     XRectangle geo;

     if(XineramaIsActive(dpy))
     {
          XineramaScreenInfo *xsi;

          xsi = XineramaQueryScreens(dpy, &n);
          geo.x = xsi[s].x_org + BORDH;
          geo.y = (conf.bartop)
               ? xsi[s].y_org + INFOBARH + TBARH
               : xsi[s].y_org + TBARH;
          geo.height = xsi[s].height - INFOBARH - TBARH;
          geo.width = xsi[s].width;

          XFree(xsi);
     }
     else
     {
          geo.x = BORDH;
          geo.y = (conf.bartop) ? INFOBARH + TBARH : TBARH;
          geo.height = MAXH - INFOBARH - TBARH;
          geo.width = MAXW;
     }

     return geo;
}

/** Get and set the selected screen
 *\return The number of the selected screen
*/
int
screen_get_sel(void)
{
     if(XineramaIsActive(dpy))
     {
          Window w;
          uint du;
          int x, y, d, i = 0;

          XQueryPointer(dpy, root, &w, &w, &x, &y, &d, &d, &du);

          for(i = 0; i < screen_count(); ++i)
               if((x >= screen_get_geo(i).x && x < screen_get_geo(i).x + screen_get_geo(i).width)
                  && (y >= screen_get_geo(i).y - INFOBARH - TBARH
                      && y < screen_get_geo(i).y - INFOBARH - TBARH + screen_get_geo(i).height + INFOBARH))
                    selscreen = i;

          return selscreen;
     }
     else
          return 0;
}

/** Init screen geo
 */
void
screen_init_geo(void)
{
     int i;

     sgeo = emalloc(screen_count(), sizeof(XRectangle));

     for(i = 0; i < screen_count(); ++i)
          sgeo[i] = screen_get_geo(i);

     return;
}


