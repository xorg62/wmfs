/*
*      titlebar.c
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
titlebar_create(Client *c)
{
     int y;

     /* Set titlebar position : Top/Bottom */
     switch(conf.titlebar.pos)
     {
     case Bottom:
          y = c->geo.y + c->geo.height + conf.client.borderheight;
          break;
     default:
     case Top:
          y = c->geo.y - (conf.titlebar.height + conf.client.borderheight);
          break;
     }

     c->tbar = bar_create(c->geo.x, y, c->geo.width,
                          conf.titlebar.height - conf.client.borderheight,
                          conf.client.borderheight,
                          conf.titlebar.bg_normal, True);
     XSetWindowBorder(dpy, c->tbar->win, conf.titlebar.bg_normal);

     return;
}

void
titlebar_delete(Client *c)
{
     bar_delete(c->tbar);
     c->tbar = NULL;

     return;
}


Client*
titlebar_get(Window w)
{
     Client *c;

     if(!conf.titlebar.exist)
          return NULL;

     for(c = clients; c && c->tbar->win != w; c = c->next);

     return c;
}

void
titlebar_update_position(Client *c)
{
     int y;

     /* Set titlebar position : Top/Bottom */
     switch(conf.titlebar.pos)
     {
     default:
     case Top:
          y = c->geo.y - conf.titlebar.height;
          break;
     case Bottom:
          y = c->geo.y + c->geo.height + conf.client.borderheight;
          break;
     }

     bar_moveresize(c->tbar, c->geo.x, y, c->geo.width,
                    conf.titlebar.height - conf.client.borderheight);

     return;
}

void
titlebar_update(Client *c)
{
     int pos_y, pos_x;
     uint bg;
     char *fg;

     XFetchName(dpy, c->win, &(c->title));
     if(!c->title)
          c->title = strdup("WMFS");

     if(!conf.titlebar.exist)
          return;

     /* Set titlebar color */
     bg = (c == sel)
          ? conf.titlebar.bg_focus
          : conf.titlebar.bg_normal;
     fg = (c == sel)
          ? conf.titlebar.fg_focus
          : conf.titlebar.fg_normal;
     c->tbar->color = bg;

     /* Refresh titlebar color */
     bar_refresh_color(c->tbar);

     /* Draw the client title in the titlebar *logeek* */
     if(conf.titlebar.height + 1 > fonth)
     {
          /* Set the text alignement */
          switch(conf.titlebar.text_align)
          {
          case Center:
               pos_x = (c->geo.width / 2) - (textw(c->title) / 2);
               break;
          case Right:
               pos_x = c->geo.width - textw(c->title) - 2;
               break;
          default:
          case Left:
               pos_x = 2;
               break;
          }

          /* Set y text position (always at the middle) and fg color */
          pos_y = (fonth - (xftfont->descent )) + ((conf.titlebar.height - fonth) / 2);

          /* Draw title */
          draw_text(c->tbar->dr, pos_x, pos_y, fg, bg, 0, c->title);
     }

     bar_refresh(c->tbar);

     return;
}
