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
     int ocx, ocy, mx, my, i;
     int dint, oscreen = c->screen;
     uint duint;
     Window dw, sw;
     Client *sclient;
     XRectangle geo = c->geo, ogeo;
     XEvent ev;

     if(c->max || c->lmax || c->state_fullscreen || c->state_dock)
          return;

     ocx =  c->geo.x;
     ocy =  c->geo.y;

     if(XGrabPointer(dpy, ROOT, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurMove], CurrentTime) != GrabSuccess)
          return;

     XQueryPointer(dpy, ROOT, &dw, &dw, &mx, &my, &dint, &dint, &duint);

     do
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);
          screen_get_sel();

          if(ev.type == MotionNotify)
          {
               if(c->tile)
               {
                    XQueryPointer(dpy, ROOT, &dw, &sw, &mx, &my, &dint, &dint, &duint);

                    /* To move the client in the tile grid */
                    if((sclient = client_gb_win(sw))
                       || (sclient = client_gb_frame(sw))
                       || (sclient = client_gb_titlebar(sw)))
                    {
                         ogeo = c->geo;
                         client_moveresize(c, sclient->geo, False);
                         client_moveresize(sclient, ogeo, False);
                    }

                    /* To move a client of one tag to another */
                    XQueryPointer(dpy, infobar[selscreen].bar->win, &dw, &sw, &mx, &my, &dint, &dint, &duint);

                    for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
                         if(infobar[selscreen].tags[i]->win == sw)
                         {
                              c->screen = selscreen;
                              c->tag = i;
                              arrange(selscreen);
                              if(c->screen != oscreen)
                                   arrange(oscreen);
                         }
               }

               /* To move a client normally, in freelayout */
               else
               {
                    geo.x = (ocx + (ev.xmotion.x - mx));
                    geo.y = (ocy + (ev.xmotion.y - my));

                    client_moveresize(c, geo, False);
               }
          }
          else if(ev.type == MapRequest
                  || ev.type == Expose
                  || ev.type == ConfigureRequest)
               getevent(ev);
     }
     while(ev.type != ButtonRelease);

     XUngrabPointer(dpy, CurrentTime);

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
     XRectangle geo = c->geo;
     XEvent ev;
     Window w;
     int d, u, omx, omy;
     float mwf = tags[selscreen][seltag[selscreen]].mwfact;

     if(c->max || c->lmax
        || c->state_fullscreen || c->state_dock)
          return;

     XQueryPointer(dpy, ROOT, &w, &w, &omx, &omy, &d, &d, (uint *)&u);

     if(XGrabPointer(dpy, ROOT, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurResize], CurrentTime) != GrabSuccess)
          return;

     if(!c->tile)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width + conf.client.borderheight, c->geo.height);

     do
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);

          if(ev.type == MotionNotify)
          {
               if(c->tile && tags[selscreen][seltag[selscreen]].layout.func != grid)
               {
                    if(tags[selscreen][seltag[selscreen]].layout.func == tile)
                         mwf += (ROUND(ev.xmotion.x_root) - omx) / (sgeo[c->screen].width);
                    else if(tags[selscreen][seltag[selscreen]].layout.func == tile_left)
                         mwf -= (ROUND(ev.xmotion.x_root) - omx) / (sgeo[c->screen].width);
                    else if(tags[selscreen][seltag[selscreen]].layout.func == tile_top)
                         mwf -= (ROUND(ev.xmotion.y_root) - omy) / (sgeo[c->screen].height);
                    else
                         mwf += (ROUND(ev.xmotion.y_root) - omy) / (sgeo[c->screen].height);

                    omx = ROUND(ev.xmotion.x_root);
                    omy = ROUND(ev.xmotion.y_root);

                    tags[selscreen][seltag[selscreen]].mwfact = (mwf < 0.05) ? 0.05 : ((mwf > 0.95) ? 0.95 : mwf);
               }
               else if(!c->tile)
               {
                    geo.width = ((ev.xmotion.x - ocx < 1) ? 1 : ev.xmotion.x - ocx);
                    geo.height = ((ev.xmotion.y - ocy < 1) ? 1 : ev.xmotion.y - ocy);

                    if(!conf.resize_transparent)
                         client_moveresize(c, geo, True);

                    XSync(dpy, False);
               }
          }
          else if(ev.type == Expose)
               expose(&ev.xexpose);

     }
     while(ev.type != ButtonRelease);

     if(!c->tile)
     {
          client_moveresize(c, geo, True);
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width + conf.client.borderheight, c->geo.height);
     }
     else
          tags[selscreen][seltag[selscreen]].layout.func(c->screen);

     XUngrabPointer(dpy, CurrentTime);


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
          for(i = 0; i < LEN(but); ++i)
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


