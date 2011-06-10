/*
*      frame.c
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

/** Frame creation function, for make a
 *  client frame, and configure it
 * \param c Client pointer
*/
void
frame_create(Client *c)
{
     XSetWindowAttributes at;
     int i;

     at.background_pixel  = conf.client.bordernormal;
     at.background_pixmap = ParentRelative;
     at.override_redirect = True;
     at.bit_gravity       = StaticGravity;
     at.event_mask        = SubstructureRedirectMask|SubstructureNotifyMask
          |ExposureMask|VisibilityChangeMask
          |EnterWindowMask|LeaveWindowMask|FocusChangeMask
          |KeyMask|ButtonMask|MouseMask;

     /* Set property */
     c->frame_geo.x          = c->geo.x - BORDH;
     c->frame_geo.y          = c->geo.y - TBARH;
     c->frame_geo.width      = FRAMEW(c->geo.width);
     c->frame_geo.height     = FRAMEH(c->geo.height);
     c->colors.fg            = conf.titlebar.fg_normal;
     c->colors.frame         = conf.client.bordernormal;
     c->colors.resizecorner  = conf.client.resizecorner_normal;

     /* Create frame window */
     CWIN(c->frame, ROOT,
          c->frame_geo.x,
          c->frame_geo.y,
          c->frame_geo.width,
          c->frame_geo.height, 0,
          CWOverrideRedirect | CWBackPixmap | CWEventMask,
          c->colors.frame, &at);


     /* Create titlebar window */
     if(TBARH - BORDH)
     {
          c->titlebar = barwin_create(c->frame, 0, 0,
                                      c->frame_geo.width ,
                                      TBARH,
                                      c->colors.frame,
                                      c->colors.fg,
                                      True, conf.titlebar.stipple.active, False);

          /* Buttons */
          if(BUTTONWH >= 1)
          {
               c->button = xcalloc(conf.titlebar.nbutton, sizeof(Window));
               for(i = 0; i < conf.titlebar.nbutton; ++i)
               {
                    CWIN(c->button[i], c->titlebar->win,
                         (c->button_last_x = (BORDH + (BUTTONWH * i) + (i << 2))),
                         ((BUTTONWH - 1) >> 1), BUTTONWH, BUTTONWH,
                         1, CWEventMask|CWOverrideRedirect|CWBackPixmap,
                         c->colors.frame, &at);

                    XSetWindowBorder(dpy, c->button[i], getcolor(c->colors.fg));
               }
          }
     }

     at.event_mask &= ~(EnterWindowMask | LeaveWindowMask); /* <- Delete useless mask */

     /* Create resize area */
     at.cursor = cursor[CurRightResize];
     CWIN(c->resize[Right], c->frame,
          c->frame_geo.width - RESHW,
          c->frame_geo.height - RESHW,
          RESHW, RESHW, 0,
          CWEventMask | CWBackPixel | CWCursor,
          c->colors.resizecorner, &at);

     at.cursor = cursor[CurLeftResize];
     CWIN(c->resize[Left], c->frame,
          0, c->frame_geo.height - RESHW,
          RESHW, RESHW, 0,
          CWEventMask | CWBackPixel | CWCursor,
          c->colors.resizecorner, &at);

     /* Border (for shadow) */
     if(conf.client.border_shadow)
     {
          c->colors.borddark  = color_shade(c->colors.frame, conf.colors.client_dark_shade);
          c->colors.bordlight = color_shade(c->colors.frame, conf.colors.client_light_shade);

          CWIN(c->left, c->frame, 0, 0, SHADH, c->frame_geo.height, 0, CWBackPixel,  c->colors.bordlight, &at);
          CWIN(c->top, c->frame, 0, 0, c->frame_geo.width, SHADH, 0, CWBackPixel, c->colors.bordlight, &at);
          CWIN(c->bottom, c->frame, 0, c->frame_geo.height - SHADH, c->frame_geo.width, SHADH, 0, CWBackPixel, c->colors.borddark, &at);
          CWIN(c->right, c->frame, c->frame_geo.width - SHADH, 0, SHADH, c->frame_geo.height, 0, CWBackPixel, c->colors.borddark, &at);
     }

     /* Reparent window with the frame */
     XReparentWindow(dpy, c->win, c->frame, BORDH, TBARH);

     return;
}

/** Delete a frame
 * \param c The client frame
*/
void
frame_delete(Client *c)
{
     /* If there is, delete the titlebar */
     if(TBARH - BORDH)
     {
          barwin_delete_subwin(c->titlebar);
          barwin_delete(c->titlebar);
     }

     /* Delete the frame's sub win and the frame */
     XDestroySubwindows(dpy, c->frame);
     XDestroyWindow(dpy, c->frame);

     return;
}

/** Move a frame
 * \param c The client frame
 * \param geo Coordinate info for move the frame
*/
void
frame_moveresize(Client *c, Geo geo)
{
     CHECK(c);

     c->frame_geo.x      = (geo.x) ? geo.x - BORDH : c->frame_geo.x;
     c->frame_geo.y      = (geo.y) ? geo.y - TBARH : c->frame_geo.y;
     c->frame_geo.width  = (geo.width)  ? FRAMEW(geo.width)  : c->frame_geo.width;
     c->frame_geo.height = (geo.height) ? FRAMEH(geo.height) : c->frame_geo.height;

     /* Frame */
     XMoveResizeWindow(dpy, c->frame,
                       c->frame_geo.x,
                       c->frame_geo.y,
                       c->frame_geo.width,
                       c->frame_geo.height);

     /* Titlebar */
     if(TBARH - BORDH)
          barwin_resize(c->titlebar, c->frame_geo.width, TBARH);

     /* Resize area */
     XMoveWindow(dpy, c->resize[Right], c->frame_geo.width - RESHW, c->frame_geo.height - RESHW);
     XMoveWindow(dpy, c->resize[Left], 0, c->frame_geo.height - RESHW);

     /* Border */
     if(conf.client.border_shadow)
     {
          XResizeWindow(dpy, c->left, SHADH, c->frame_geo.height - SHADH);
          XResizeWindow(dpy, c->top, c->frame_geo.width, SHADH);
          XMoveResizeWindow(dpy, c->bottom, 0, c->frame_geo.height - SHADH, c->frame_geo.width, SHADH);
          XMoveResizeWindow(dpy, c->right, c->frame_geo.width - SHADH, 0, SHADH, c->frame_geo.height);
     }

     return;
}

/** Update frame colors for focus event
  *\param c Client pointer
*/
void
frame_update_color(Client *c, bool focused)
{
     CHECK(c);

     /* Not focused client */
     if(focused)
     {
          c->colors.frame        = conf.client.bordernormal;
          c->colors.fg           = conf.titlebar.fg_normal;
          c->colors.resizecorner = conf.client.resizecorner_normal;

          if(TBARH - BORDH)
               c->titlebar->stipple_color = conf.titlebar.stipple.colors.normal;
     }
     /* Focused */
     else
     {
          c->colors.frame        = conf.client.borderfocus;
          c->colors.fg           = conf.titlebar.fg_focus;
          c->colors.resizecorner = conf.client.resizecorner_focus;

          if(TBARH - BORDH)
               c->titlebar->stipple_color = conf.titlebar.stipple.colors.focus;
     }

     if(conf.client.border_shadow)
     {
          c->colors.borddark  = color_shade(c->colors.frame, conf.colors.client_dark_shade);
          c->colors.bordlight = color_shade(c->colors.frame, conf.colors.client_light_shade);
     }

     return;
}

/** Update the client frame; Set the new color
 *  and the title --> refresh
 * \param c Client pointer
*/
void
frame_update(Client *c)
{
     int i;

     CHECK(c);

     if(TBARH - BORDH)
     {
          c->titlebar->bg = c->colors.frame;
          c->titlebar->fg = c->colors.fg;

          barwin_refresh_color(c->titlebar);

          /* Buttons */
          if(conf.titlebar.nbutton && BUTTONWH >= 1)
          {
               if(conf.titlebar.stipple.active)
                    draw_rectangle(c->titlebar->dr, 0, 0, c->button_last_x + TBARH - (TBARH >> 2),
                                   TBARH + (BORDH << 2), c->colors.frame);

               for(i = 0; i < conf.titlebar.nbutton; ++i)
               {
                    XSetWindowBackground(dpy, c->button[i], c->colors.frame);
                    XClearWindow(dpy, c->button[i]);

                    if((!conf.titlebar.button[i].flags)
                              || ((conf.titlebar.button[i].flags & FreeFlag)
                                   && ((c->flags & FreeFlag) || !(c->flags & (TileFlag | LMaxFlag))))
                              || ((conf.titlebar.button[i].flags & MaxFlag)
                                   && ((c->flags & MaxFlag) || (c->flags & LMaxFlag)))
                              || ((conf.titlebar.button[i].flags & TileFlag) && (c->flags & TileFlag)))
                    {

                         XSetWindowBorder(dpy, c->button[i], getcolor(c->colors.fg));

                         /* Button's lines */
                         if(conf.titlebar.button[i].nlines)
                         {
                              XSetForeground(dpy, gc, getcolor(c->colors.fg));
                              XDrawSegments(dpy, c->button[i], gc,
                                        conf.titlebar.button[i].linecoord,
                                        conf.titlebar.button[i].nlines);
                         }
                    }
                    else
                         XSetWindowBorder(dpy, c->button[i], c->colors.frame);
               }
          }

          barwin_refresh(c->titlebar);
     }

     XSetWindowBackground(dpy, c->frame, c->colors.frame);
     XSetWindowBackground(dpy, c->resize[Right], c->colors.resizecorner);
     XSetWindowBackground(dpy, c->resize[Left], c->colors.resizecorner);
     XClearWindow(dpy, c->frame);
     XClearWindow(dpy, c->resize[Right]);
     XClearWindow(dpy, c->resize[Left]);


     if(conf.client.border_shadow)
     {
          XSetWindowBackground(dpy, c->left,   c->colors.bordlight);
          XSetWindowBackground(dpy, c->top,    c->colors.bordlight);
          XSetWindowBackground(dpy, c->right,  c->colors.borddark);
          XSetWindowBackground(dpy, c->bottom, c->colors.borddark);

          XClearWindow(dpy, c->left);
          XClearWindow(dpy, c->top);
          XClearWindow(dpy, c->right);
          XClearWindow(dpy, c->bottom);
     }

     if(TBARH - BORDH)
          barwin_draw_text(c->titlebar,
                    (c->frame_geo.width >> 1) - (textw(c->title) >> 1),
                    ((font->height - font->descent) + ((TBARH - font->height) >> 1)),
                    c->title);

     return;
}


