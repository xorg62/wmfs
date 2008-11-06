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

void
mouse_move(Client *c)
{
     int ocx = c->geo.x;
     int ocy = c->geo.y;
     int mx, my, dint;
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
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);

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
               expose(ev);
     }

     return;
}

void
mouse_resize(Client *c)
{
     int ocx = c->geo.x;
     int ocy = c->geo.y;
     int my = sgeo.y, mx = 0;
     double fy, fx, mwf;
     XRectangle geo;
     XEvent ev;

    if(c->max || c->lmax)
          return;

     if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurResize], CurrentTime) != GrabSuccess)
          return;

     if(!c->tile)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width, c->geo.height);

     /* Warp pointer for mwfact resize {{{ */
     if(c->tile)
     {
          if(tags[seltag].layout.func == tile)
               mx = tags[seltag].mwfact * sgeo.width;
          else if(tags[seltag].layout.func == tile_left)
               mx = sgeo.width - (tags[seltag].mwfact * sgeo.width);
          else if(tags[seltag].layout.func == tile_top)
          {
               mx = event.xmotion.x_root;
               my = sgeo.height - (tags[seltag].mwfact * sgeo.height);
          }
          else if(tags[seltag].layout.func == tile_bottom)
          {
               mx = event.xmotion.x_root;
               my = tags[seltag].mwfact * sgeo.height;
          }
          XWarpPointer(dpy, None, root, 0, 0, 0, 0, mx, my);
     }
     /* }}} */

     for(;;)
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);

          if(ev.type == ButtonRelease)
          {
               if(!c->tile)
                    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width, c->geo.height);
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
                                  fy = (round(((ev.xmotion.y * 50) / sgeo.height))) / 50;
                    fx = (round(((ev.xmotion.x * 50) / sgeo.width))) / 50;

                    if(tags[seltag].layout.func == tile)
                         mwf = fx;
                    else if(tags[seltag].layout.func == tile_left)
                         mwf = 1 - fx;
                    else if(tags[seltag].layout.func == tile_top)
                         mwf = 1 - fy;
                    else if(tags[seltag].layout.func == tile_bottom)
                         mwf = fy;

                    tags[seltag].mwfact =
                         (((0.01) > (mwf) ? (0.01) : (mwf) ) < 0.99)
                         ? ((0.01) > (mwf) ? (0.01): (mwf))
                         : 0.99;

                    arrange();
               }

               if(!c->tile)
                    client_moveresize(c, geo, True);
          }
          else if(ev.type == Expose)
               expose(ev);

     }

     return;
}

void
mouse_grabbuttons(Client *c, Bool focused)
{
     int i, j;
     uint mod = conf.client.mod;
     uint bl[] = {Button1, Button2, Button3, Button4, Button5};
     uint ml[] = {mod, mod|LockMask, mod|numlockmask, mod|scrolllockmask,
                  mod|numlockmask|scrolllockmask, mod|LockMask|scrolllockmask,
                  mod|LockMask|numlockmask,mod|LockMask|numlockmask|scrolllockmask};

     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

     if(focused)
          for(i = 0; i < (sizeof bl / sizeof bl[0]); ++i)
               for(j = 0; j < (sizeof ml / sizeof ml[0]); ++j)
                    XGrabButton(dpy, bl[i], ml[j], c->win, False,
                                ButtonMask, GrabModeAsync,GrabModeSync, None, None);
     else
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
                      ButtonMask, GrabModeAsync, GrabModeSync, None, None);

     return;
}

void
uicb_mouse_move(uicb_t cmd)
{
     if(sel)
          mouse_move(sel);

     return;
}

void
uicb_mouse_resize(uicb_t cmd)
{
     if(sel)
          mouse_resize(sel);

     return;
}


