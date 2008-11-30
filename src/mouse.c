/*
*      mouse.c
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

/** Move the client with the mouse
 * \param c Client pointer
*/
void
mouse_move(Client *c)
{
     int ocx = c->geo.x;
     int ocy = c->geo.y;
     int mx = 0, my = 0, dint;
     uint duint;
     Window dw;
     XRectangle geo;
     XEvent ev;

     if(c->max || c->tile || c->lmax)
          return;

     if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurMove], CurrentTime) != GrabSuccess)
          return;

     XQueryPointer(dpy, root, &dw, &dw, &mx, &my, &dint, &dint, &duint);

     for(;;)
     {
          XMaskEvent(dpy, ButtonMask | MouseMask | ExposureMask | SubstructureRedirectMask, &ev);

          if(ev.type == ButtonRelease)
          {
               XUngrabPointer(dpy, CurrentTime);
               return;
          }
          else if(ev.type == MotionNotify)
          {
               XSync(dpy, False);
               geo.width = c->geo.width;
               geo.height = c->geo.height;
               geo.x = (ocx + (ev.xmotion.x - mx));
               geo.y = (ocy + (ev.xmotion.y - my));

               client_moveresize(c, geo, True);
          }
          else if(ev.type == Expose)
               expose(&ev.xexpose);
     }

     return;
}

/** Resize a client with the mouse
 * \param c Client pointer
*/
void
mouse_resize(Client *c)
{
     int ocx = c->geo.x;
     int ocy = c->geo.y;
     double fy, fx, mwf;
     XRectangle geo;
     XRectangle sg = screen_get_geo(screen_get_sel());
     int my = sg.y, mx = 0;
     XEvent ev;

    if(c->max || c->lmax)
          return;

     if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurResize], CurrentTime) != GrabSuccess)
          return;

     if(!c->tile)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width + conf.client.borderheight, c->geo.height);

     /* Warp pointer for mwfact resize {{{ */
     if(c->tile)
     {
          if(tags[selscreen][seltag[selscreen]].layout.func == tile)
               mx = tags[selscreen][seltag[selscreen]].mwfact * sg.width;
          else if(tags[selscreen][seltag[selscreen]].layout.func == tile_left)
               mx = sg.width - (tags[selscreen][seltag[selscreen]].mwfact * sg.width);
          else if(tags[selscreen][seltag[selscreen]].layout.func == tile_top)
          {
               mx = sg.width / 2;
               my = sg.height - (tags[selscreen][seltag[selscreen]].mwfact * sg.height);
          }
          else if(tags[selscreen][seltag[selscreen]].layout.func == tile_bottom)
          {
               mx = sg.width / 2;
               my = tags[selscreen][seltag[selscreen]].mwfact * sg.height;
          }
          XWarpPointer(dpy, None, root, 0, 0, 0, 0, sg.x + mx, sg.y + my);
     }
     /* }}} */

     for(;;)
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);

          if(ev.type == ButtonRelease)
          {
               if(!c->tile)
                    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width + conf.client.borderheight, c->geo.height);
               XUngrabPointer(dpy, CurrentTime);
               return;
          }
          else if(ev.type == MotionNotify)
          {
               XSync(dpy, False);

               if(!c->tile)
               {
                    geo.x = c->geo.x; geo.y = c->geo.y;
                    geo.width = ((ev.xmotion.x - ocx < 1) ? 1 : ev.xmotion.x - ocx);
                    geo.height = ((ev.xmotion.y - ocy < 1) ? 1 : ev.xmotion.y - ocy);
               }
               else
               {
                    fy = (round(((ev.xmotion.y * 50) / sg.height))) / 50;
                    fx = (round(((ev.xmotion.x * 50) / sg.width))) / 50;

                    if(tags[selscreen][seltag[selscreen]].layout.func == tile)
                         mwf = sg.x + fx;
                    else if(tags[selscreen][seltag[selscreen]].layout.func == tile_left)
                         mwf = 1 - (sg.x + fx);
                    else if(tags[selscreen][seltag[selscreen]].layout.func == tile_top)
                         mwf = 1 - fy;
                    else if(tags[selscreen][seltag[selscreen]].layout.func == tile_bottom)
                         mwf = fy;
                    else
                         mwf = tags[selscreen][seltag[selscreen]].mwfact;

                    tags[selscreen][seltag[selscreen]].mwfact = (mwf < 0.05) ? 0.05 : ((mwf > 0.95) ? 0.95 : mwf);
                    tags[selscreen][seltag[selscreen]].layout.func();
               }

               if(!c->tile)
                    client_moveresize(c, geo, True);
          }
          else if(ev.type == Expose)
               expose(&ev.xexpose);

     }

     return;
}

/** Grab buttons
 * \param c Client pointer
 * \param focused For know if c is or not focused
*/
void
mouse_grabbuttons(Client *c, Bool focused)
{
     int i;
     uint but[] = {Button1, Button2, Button3, Button4, Button5};

     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     if(focused)
          for(i = 0; i < (sizeof but / sizeof but[0]); ++i)
          {
               XGrabButton(dpy, but[i], conf.client.mod, c->win, False,
                           ButtonMask, GrabModeAsync,GrabModeSync, None, None);
               XGrabButton(dpy, but[i], conf.client.mod|LockMask, c->win, False,
                           ButtonMask, GrabModeAsync,GrabModeSync, None, None);
               XGrabButton(dpy, but[i], conf.client.mod|numlockmask, c->win, False,
                           ButtonMask, GrabModeAsync,GrabModeSync, None, None);
               XGrabButton(dpy, but[i], conf.client.mod|LockMask|numlockmask, c->win, False,
                           ButtonMask, GrabModeAsync,GrabModeSync, None, None);
          }
     else
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                      ButtonMask, GrabModeAsync, GrabModeSync, None, None);

     return;
}

/** Move the selected client
 * \param cmd uicb_t type unused
*/
void
uicb_mouse_move(uicb_t cmd)
{
     CHECK(sel);

     mouse_move(sel);

     return;
}

/** Reisze the selected client
 * \param cmd uicb_t type unused
*/
void
uicb_mouse_resize(uicb_t cmd)
{
     CHECK(sel);

     mouse_resize(sel);

     return;
}


