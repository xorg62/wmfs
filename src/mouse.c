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

/** Draw the border when a client in dragging/resizing with mouse
 */
void
mouse_dragborder(XRectangle geo, GC g)
{
     XDrawRectangle(dpy, ROOT, g,
                    geo.x - BORDH / 2,
                    geo.y - (TBARH - (BORDH / 2)),
                    geo.width + BORDH,
                    geo.height + TBARH);

     return;
}

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
     XRectangle geo = c->geo;
     XGCValues xgc;
     GC gci;
     XEvent ev;

     if(c->max || c->lmax || c->state_fullscreen || c->state_dock)
          return;

     ocx =  c->geo.x;
     ocy =  c->geo.y;

     if(XGrabPointer(dpy, ROOT, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[CurMove], CurrentTime) != GrabSuccess)
          return;

     if(!c->tile)
          XGrabServer(dpy);

     /* Set the GC for the rectangle */
     xgc.function = GXinvert;
     xgc.subwindow_mode = IncludeInferiors;
     xgc.line_width = BORDH;
     gci = XCreateGC(dpy, ROOT, GCFunction|GCSubwindowMode|GCLineWidth, &xgc);

     if(!c->tile)
          mouse_dragborder(c->geo, gci);

     XQueryPointer(dpy, ROOT, &dw, &dw, &mx, &my, &dint, &dint, &duint);

     do
     {
          XMaskEvent(dpy, MouseMask | SubstructureRedirectMask, &ev);
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
                         if(c != sclient)
                         {
                              client_swap(c, sclient);
                              break;
                         }
                    }

                    /* To move a client from one tag to another */
                    XQueryPointer(dpy, infobar[selscreen].bar->win, &dw, &sw, &mx, &my, &dint, &dint, &duint);

                    for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
                         if(infobar[selscreen].tags[i]->win == sw)
                         {
                              c->screen = selscreen;
                              c->tag = i;
                              tags[c->screen][c->tag].request_update = True;
                              arrange(oscreen, True);
                              if(oscreen != c->screen)
                                   arrange(c->screen, True);
                         }
               }

               /* To move a client normally, in freelayout */
               else
               {
                    mouse_dragborder(geo, gci);

                    geo.x = (ocx + (ev.xmotion.x - mx));
                    geo.y = (ocy + (ev.xmotion.y - my));

                    /*
                     * Need to draw 2 times the same rectangle because
                     * it is draw with the revert color; revert + revert = normal
                     */
                    mouse_dragborder(geo, gci);
               }
          }
          else if(ev.type == MapRequest
                  || ev.type == ConfigureRequest)
               getevent(ev);
     }
     while(ev.type != ButtonRelease);

     /* One time again to delete all the trace on the window */
     if(!c->tile)
     {
          mouse_dragborder(geo, gci);
          client_moveresize(c, geo, False);
          frame_update(c);
          XUngrabServer(dpy);
     }
     client_update_attributes(c);
     XUngrabPointer(dpy, CurrentTime);
     XFreeGC(dpy, gci);

     return;
}

/** Resize a client with the mouse
 * \param c Client pointer
*/

void
mouse_resize(Client *c)
{
     XRectangle geo = c->geo, ogeo = c->geo;
     Position pos = Right;
     XEvent ev;
     Window w;
     int d, u, omx, omy;
     XGCValues xgc;
     GC gci;
     float mwf = tags[selscreen][seltag[selscreen]].mwfact;

     if(c->max || c->lmax
        || c->state_fullscreen || c->state_dock)
          return;

     XQueryPointer(dpy, ROOT, &w, &w, &omx, &omy, &d, &d, (uint *)&u);

     if((omx - c->geo.x) < (c->geo.width / 2))
          pos = Left;

     if(XGrabPointer(dpy, ROOT, False, MouseMask, GrabModeAsync, GrabModeAsync, None,
                     cursor[((c->tile) ? CurResize : ((pos == Right) ? CurRightResize : CurLeftResize))],
                     CurrentTime) != GrabSuccess)
          return;

     if(!c->tile)
          XGrabServer(dpy);

     /* Set the GC for the rectangle */
     xgc.function = GXinvert;
     xgc.subwindow_mode = IncludeInferiors;
     xgc.line_width = BORDH;
     gci = XCreateGC(dpy, ROOT, GCFunction|GCSubwindowMode|GCLineWidth, &xgc);

     if(!c->tile)
     {
          if(pos == Right)
               XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width + conf.client.borderheight, c->geo.height);
          else
               XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, 0, c->geo.height);
          mouse_dragborder(c->geo, gci);
     }

     do
     {
          XMaskEvent(dpy, MouseMask | SubstructureRedirectMask, &ev);

          if(ev.type == MotionNotify)
          {
               /* To resize MWFACT in tile mode */
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
               /* Free mode */
               else if(!c->tile)
               {
                    mouse_dragborder(geo, gci);

                    if(pos == Right)
                    {
                         geo.width = ((ev.xmotion.x - c->geo.x < c->minw) ? c->minw : ev.xmotion.x - c->geo.x);
                         geo.height = ((ev.xmotion.y - c->geo.y < c->minh) ? c->minh : ev.xmotion.y - c->geo.y);
                    }
                    else
                    {
                         geo.x = (geo.width != c->maxw) ? c->geo.x - (c->geo.x - ev.xmotion.x) : geo.x;
                         geo.width  = ((c->geo.width + (c->geo.x - geo.x) < c->minw)
                                       ? c->minw && (geo.x = (c->geo.x + c->geo.width) - c->minw)
                                       : c->geo.width + (c->geo.x - geo.x));
                         geo.height = ((ev.xmotion.y - c->geo.y <= c->minh) ? c->minh : ev.xmotion.y - c->geo.y);
                    }

                    client_geo_hints(&geo, c);

                    mouse_dragborder((ogeo = geo), gci);

                    XSync(dpy, False);
               }
          }
     }
     while(ev.type != ButtonRelease);

     if(!c->tile)
     {
          mouse_dragborder(ogeo, gci);
          client_moveresize(c, geo, True);
          frame_update(c);
          XUngrabServer(dpy);
     }
     else
          tags[selscreen][seltag[selscreen]].layout.func(c->screen);

     client_update_attributes(c);
     XUngrabPointer(dpy, CurrentTime);
     XFreeGC(dpy, gci);

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


