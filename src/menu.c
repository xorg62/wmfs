/*
*      menu.c
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
menu_init(Menu *menu, int nitem)
{
     menu->nitem = nitem;
     menu->item = emalloc(sizeof(MenuItem), nitem);

     return;
}

void
menu_new_item(MenuItem *mi, char *name, void *func, char *cmd)
{
     mi->name = name;
     mi->func = func;
     mi->cmd = cmd;

     return;
}

void
menu_draw(Menu menu, int x, int y)
{
     int i, width;
     int r = 1;
     XEvent ev;
     BarWindow *item[menu.nitem];
     BarWindow *frame;

     width = menu_get_longer_string(menu.item, menu.nitem);

     /* Frame barwin */
     frame = barwin_create(ROOT, x, y, width + SHADH,
                           menu.nitem * INFOBARH + SHADH,
                           conf.colors.bar, conf.colors.text,
                           False, False, True);

     barwin_map(frame);
     barwin_map_subwin(frame);
     barwin_refresh_color(frame);

     for(i = 0; i < menu.nitem; ++i)
     {
          item[i] = barwin_create(frame->win,
                                  SHADH,
                                  (i * INFOBARH) + SHADH,
                                  width - SHADH,
                                  INFOBARH,
                                  conf.colors.bar,
                                  conf.colors.text,
                                  True, False, False);
          barwin_map(item[i]);
          barwin_map_subwin(frame);
          barwin_refresh_color(item[i]);
          barwin_refresh(item[i]);

          barwin_draw_text(item[i], ((width / 2) - (textw(menu.item[i].name) / 2)), font->height, menu.item[i].name);
     }

     while(r)
     {
          switch(ev.type)
          {
          case ButtonPress:
               /* Execute the function linked with the item */
               for(i = 0; i < menu.nitem; ++i)
                    if(ev.xbutton.window == item[i]->win
                       && ev.xbutton.button == Button1)
                         if(menu.item[i].func)
                              menu.item[i].func(menu.item[i].cmd);
               r = 0;
               break;
          case EnterNotify:
               /* For focus an item with the mousw */
               for(i = 0; i < menu.nitem; ++i)
               {
                    if(ev.xcrossing.window == item[i]->win)
                    {
                         item[i]->fg = conf.colors.tagselfg;
                         item[i]->bg = conf.colors.tagselbg;
                    }
                    else
                    {
                         item[i]->fg = conf.colors.text;
                         item[i]->bg = conf.colors.bar;
                    }

                    barwin_refresh_color(item[i]);
                    barwin_draw_text(item[i], ((width / 2) - (textw(menu.item[i].name) / 2)), font->height, menu.item[i].name);
                    barwin_refresh(item[i]);
               }
               break;
          default:
               getevent(ev);
               break;
          }
          XNextEvent(dpy, &ev);
     }

     for(i = 0; i < menu.nitem; ++i)
          barwin_delete(item[i]);
     barwin_delete(frame);

     return;
}

int
menu_get_longer_string(MenuItem *mt, int nitem)
{
     int i, l = 0;

     for(i = 0; i < nitem; ++i)
          if(textw(mt[i].name) > l)
               l = textw(mt[i].name);

     return l + PAD;
}
