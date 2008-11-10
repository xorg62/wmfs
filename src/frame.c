/*
*      frame.c
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

/** Frame creation function, for make a
 *  client frame, and configure it
 * \param c Client pointer
*/
void
frame_create(Client *c)
{
     XSetWindowAttributes at;
     int i;

     at.background_pixel = conf.client.bordernormal;
     at.background_pixmap = ParentRelative;
     at.override_redirect = True;
     at.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ExposureMask|
          VisibilityChangeMask|EnterWindowMask|FocusChangeMask|KeyMask|ButtonMask|MouseMask;

     /* Set size */
     c->frame_geo.x          =  c->geo.x - BORDH;
     c->frame_geo.y          =  c->geo.y - TBARH;
     c->frame_geo.width      =  FRAMEW(c->geo.width);
     c->frame_geo.height     =  FRAMEH(c->geo.height);
     c->colors.frame         =  conf.client.bordernormal;
     c->colors.resizecorner  =  conf.client.resizecorner_normal;

     /* Create frame window */
     CWIN(c->frame, root,
          c->frame_geo.x,
          c->frame_geo.y,
          c->frame_geo.width,
          c->frame_geo.height, 0,
          CWOverrideRedirect|CWBackPixmap|CWEventMask, c->colors.frame, &at);

     /* Create titlebar window */
     if(TBARH)
          CWIN(c->titlebar, c->frame, 0, 0,
               c->frame_geo.width,
               TBARH + BORDH,
               0, CWEventMask|CWBackPixel,
               c->colors.frame, &at);

     /* Titlebar buttons */
     at.event_mask &= ~EnterWindowMask; /* <- Delete the EnterWindow mask */
     if(CTBAR)
          for(i = 0; i < LastButton; ++i)
          {
               CWIN(c->button[i], c->frame,
                    BUTX(i), 2,
                    BUTHW, BUTHW,
                    1, CWEventMask|CWBackPixel,
                    c->colors.frame, &at);
               XSetWindowBorder(dpy, c->button[i], getcolor(conf.titlebar.fg));
          }

     /* Create resize area */
     at.cursor = cursor[CurResize];
     if(BORDH)
          CWIN(c->resize, c->frame,
               c->frame_geo.width - RESHW,
               c->frame_geo.height - RESHW,
               RESHW,
               RESHW, 0,
               CWEventMask|CWBackPixel|CWCursor, c->colors.resizecorner, &at);

     /* Reparent window with the frame */
     XReparentWindow(dpy, c->win, c->frame, BORDH, BORDH + TBARH);
     return;
}

/** Move a frame
 * \param c The client frame
 * \param geo Coordinate info for move the frame
*/
void
frame_moveresize(Client *c, XRectangle geo)
{
     c->frame_geo.x      =  geo.x - BORDH;
     c->frame_geo.y      =  geo.y - TBARH;
     c->frame_geo.width  =  FRAMEW(geo.width);
     c->frame_geo.height =  FRAMEH(geo.height);

     /* Frame */
     XMoveResizeWindow(dpy, c->frame,
                       c->frame_geo.x,
                       c->frame_geo.y,
                       c->frame_geo.width,
                       c->frame_geo.height);
     /* Titlebar */
     if(TBARH)
          XResizeWindow(dpy, c->titlebar, c->frame_geo.width, TBARH + BORDH);

     /* Resize area */
     if(BORDH)
          XMoveWindow(dpy, c->resize, c->frame_geo.width - RESHW, c->frame_geo.height - RESHW);

     return;
}

/** Update a frame
 * \param c Client pointer
*/
void
frame_update(Client *c)
{
     int i;

     if(CTBAR)
          for(i = 0; i < LastButton; ++i)
          {
               XSetWindowBackground(dpy, c->button[i], c->colors.frame);
               XClearWindow(dpy, c->button[i]);
          }
     if(BORDH)
     {
          XSetWindowBackground(dpy, c->resize, c->colors.resizecorner);
          XClearWindow(dpy, c->resize);
     }
     if(TBARH)
     {
          XSetWindowBackground(dpy, c->titlebar, c->colors.frame);
          XClearWindow(dpy, c->titlebar);
     }

     XSetWindowBackground(dpy, c->frame, c->colors.frame);
     XClearWindow(dpy, c->frame);

     if((TBARH + BORDH + 1) > font->height)
          draw_text(c->titlebar,
                    (c->frame_geo.width / 2) - (textw(c->title) / 2),
                    (font->height - (font->descent )) + (((TBARH + BORDH) - font->height) / 2),
                    conf.titlebar.fg, c->colors.frame, 0, c->title);
     return;
}


