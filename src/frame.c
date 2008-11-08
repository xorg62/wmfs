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

void
frame_create(Client *c)
{
     XSetWindowAttributes at;

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
     c->colors.titlebar      =  conf.titlebar.fg_normal;

     /* Create frame window */
     c->frame = XCreateWindow(dpy, root,
                              c->frame_geo.x,
                              c->frame_geo.y,
                              c->frame_geo.width,
                              c->frame_geo.height, 0,
                              CopyFromParent, InputOutput, CopyFromParent,
                              CWOverrideRedirect|CWBackPixmap|CWEventMask, &at);

     /* Create titlebar window */
     c->titlebar = XCreateWindow(dpy, c->frame, 0, 0,
                                 c->frame_geo.width,
                                 TBARH + BORDH, 0,
                                 CopyFromParent, InputOutput, CopyFromParent,
                                 CWEventMask|CWBackPixel, &at);

     /* Create resize area */
     at.cursor = cursor[CurResize];
     c->resize = XCreateWindow(dpy, c->frame,
                               c->frame_geo.width - RESHW,
                               c->frame_geo.height - RESHW,
                               RESHW, RESHW, 0, CopyFromParent,
                               InputOutput, CopyFromParent,
                               CWEventMask|CWBackPixel|CWCursor, &at);

     /* Color it */
     XSetWindowBackground(dpy, c->resize, c->colors.resizecorner);
     XSetWindowBackground(dpy, c->titlebar, c->colors.frame);
     XSetWindowBackground(dpy, c->frame, c->colors.frame);

     /* Reparent window with the frame */
     XReparentWindow(dpy, c->win, c->frame, BORDH, BORDH + TBARH);

     return;
}

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
     XResizeWindow(dpy, c->titlebar, c->frame_geo.width, TBARH + BORDH);

     /* Resize area */
     XMoveWindow(dpy, c->resize, c->frame_geo.width - RESHW, c->frame_geo.height - RESHW);

     return;
}

void
frame_update(Client *c)
{
     int px, py;

     XSetWindowBackground(dpy, c->frame, c->colors.frame);
     XSetWindowBackground(dpy, c->titlebar, c->colors.frame);
     XSetWindowBackground(dpy, c->resize, c->colors.resizecorner);

     XClearWindow(dpy, c->frame);
     XClearWindow(dpy, c->titlebar);
     XClearWindow(dpy, c->resize);

     /* Draw the client title \in the titlebar *logeek* */
     if((conf.titlebar.height + BORDH + 1) > font->height)
     {
          /* x position of the text (center) */
          px = (c->frame_geo.width / 2) - (textw(c->title) / 2);
          /* y position of the text (center too) */
          py = (font->height - (font->descent )) + (((TBARH + BORDH) - font->height) / 2);

          draw_text(c->titlebar, px, py, c->colors.titlebar, c->colors.frame, 0, c->title);
     }

     return;
}

Client*
frame_get(Window w)
{
     Client *c;

     for(c = clients; c && c->frame != w; c = c->next);

     return c;
}

Client*
frame_get_titlebar(Window w)
{
     Client *c;

     for(c = clients; c && c->titlebar != w; c = c->next);

     return c;
}

Client*
frame_get_resize(Window w)
{
     Client *c;

     for(c = clients; c && c->resize != w; c = c->next);

     return c;
}

