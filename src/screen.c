/*
*      screen.c
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

/** Count the screens
 *\return the number of screen
*/
int
screen_count(void)
{
     int n = 0;

     n = ScreenCount(dpy);

#ifdef HAVE_XINERAMA
     if(XineramaIsActive(dpy))
          XineramaQueryScreens(dpy, &n);
#endif /* HAVE_XINERAMA */

     /* Set _WMFS_SCREEN_COUNT */
     if(net_atom)
          XChangeProperty(dpy, ROOT, net_atom[wmfs_screen_count], XA_CARDINAL, 32,
                    PropModeReplace, (uchar*)&n, 1);

     return n;
}

/** Get screen geometry by number
 *\param s Screen number
 *\return XRectangle struct
*/
XRectangle
screen_get_geo(int s)
{
     int barpos = tags[selscreen][seltag[selscreen]].barpos;
     XRectangle geo;

     geo.x = BORDH;
     if(barpos == IB_Hide || barpos == IB_Bottom)
          geo.y = TBARH;
     else
          geo.y = INFOBARH + TBARH;
     geo.height = MAXH - INFOBARH - TBARH;
     geo.width = MAXW;

#ifdef HAVE_XINERAMA
     if(XineramaIsActive(dpy))
     {
          XineramaScreenInfo *xsi;
          int n = 0;

          xsi = XineramaQueryScreens(dpy, &n);

          geo.x = xsi[s].x_org + BORDH;
          if(barpos == IB_Hide || barpos == IB_Bottom)
               geo.y = TBARH;
          else
               geo.y = xsi[s].y_org + INFOBARH + TBARH;
          geo.height = xsi[s].height - INFOBARH - TBARH;
          geo.width = xsi[s].width;

          XFree(xsi);
     }
#endif /* HAVE_XINERAMA */

     return geo;
}

/** Get the current screen number with
 * coordinated
 *\param geo Geometry for get the screen number
 *\return The screen number
*/
int
screen_get_with_geo(int x, int y)
{
     int i, r = 0;

     for(i = 0; i < screen_count(); ++i)
          if((x >= spgeo[i].x && x < spgeo[i].x + spgeo[i].width)
             && y >= spgeo[i].y && y < spgeo[i].y + spgeo[i].height)
               r = i;

     return r;
}

/** Set the selected screen
 *\param screen Number of the wanted selected screen
*/
void
screen_set_sel(int screen)
{
     if(screen < 0 || screen > screen_count() - 1)
          screen = 0;

     client_focus(NULL);
     XWarpPointer(dpy, None, ROOT, 0, 0, 0, 0,
                  sgeo[screen].x + sgeo[screen].width / 2,
                  sgeo[screen].y + sgeo[screen].height / 2);
     selscreen = screen;

     return;
}

/** Get and set the selected screen
 *\return The number of the selected screen
*/
int
screen_get_sel(void)
{
     selscreen = 0;

#ifdef HAVE_XINERAMA
     if(XineramaIsActive(dpy))
     {
          /* Unused variables (except x/y) */
          Window w;
          int d, x, y;

          XQueryPointer(dpy, ROOT, &w, &w, &x, &y, &d, &d, (uint *)&d);

          selscreen = screen_get_with_geo(x, y);
     }
#endif /* HAVE_XINERAMA */

     /* Set _WMFS_CURRENT_SCREEN */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_current_screen], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&selscreen, 1);

     return selscreen;
}

/** Init screen geo
 */
void
screen_init_geo(void)
{
     int i;

     sgeo = emalloc(screen_count(), sizeof(XRectangle));
     spgeo = emalloc(screen_count(), sizeof(XRectangle));

     for(i = 0; i < screen_count(); ++i)
          sgeo[i] = screen_get_geo(i);

     spgeo[0].x = 0;
     spgeo[0].y = 0;
     spgeo[0].width = MAXW;
     spgeo[0].height = MAXH;

#ifdef HAVE_XINERAMA
     XineramaScreenInfo *xsi;
     int n;

     if(XineramaIsActive(dpy))
     {
          xsi = XineramaQueryScreens(dpy, &n);
          for(i = 0; i < n; ++i)
          {
               spgeo[i].x = xsi[i].x_org;
               spgeo[i].y = xsi[i].y_org;
               spgeo[i].width = xsi[i].width;
               spgeo[i].height = xsi[i].height;
          }
          XFree(xsi);
     }
#endif /* HAVE_XINERAMA */

#ifdef HAVE_XRANDR
     /* Init xrandr stuff */
     int d;

     XRRSelectInput(dpy, ROOT, 1);
     XRRQueryExtension(dpy, &xrandr_event, &d);
#endif /* HAVE_XRANDR */

     ewmh_set_desktop_geometry();
     ewmh_set_workarea();

     return;
}

/** Uicb: screen select
 * \param cmd Screen uicb_t type
*/
void
uicb_screen_select(uicb_t cmd)
{
     screen_set_sel(atoi(cmd));

     return;
}

/** Uicb: screen next
 * \param cmd Screen uicb_t type
*/
void
uicb_screen_next(uicb_t cmd)
{
     screen_get_sel();

     selscreen = (selscreen + 1 > screen_count() - 1) ? 0 : selscreen + 1;

     screen_set_sel(selscreen);

     return;
}

/** Uicb: screen prev
 * \param cmd Screen uicb_t type
*/
void
uicb_screen_prev(uicb_t cmd)
{
     screen_get_sel();

     selscreen = (selscreen - 1 < 0) ? screen_count() - 1 : selscreen - 1;

     screen_set_sel(selscreen);

     return;
}
