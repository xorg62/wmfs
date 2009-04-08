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
menu_init(Menu *menu, char *name, int nitem, uint bg_f, char *fg_f, uint bg_n, char *fg_n)
{

     /* Item */
     menu->nitem = nitem;
     menu->item = emalloc(sizeof(MenuItem), nitem);
     menu->name = name;

     /* Colors */
     menu->colors.focus.bg = bg_f;
     menu->colors.focus.fg = fg_f;
     menu->colors.normal.bg = bg_n;
     menu->colors.normal.fg = fg_n;

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
     XEvent ev;
     BarWindow *item[menu.nitem];
     BarWindow *frame;

     width = menu_get_longer_string(menu.item, menu.nitem);

     /* Frame barwin */
     frame = barwin_create(ROOT, x, y, width + SHADH, menu.nitem * (INFOBARH - SHADH) + SHADH * 2,
                           menu.colors.normal.bg, menu.colors.normal.fg, False, False, True);

     barwin_map(frame);
     barwin_map_subwin(frame);
     barwin_refresh_color(frame);

     for(i = 0; i < menu.nitem; ++i)
     {
          item[i] = barwin_create(frame->win,
                                  SHADH,
                                  (i * (INFOBARH - SHADH) + SHADH),
                                  width - SHADH,
                                  INFOBARH,
                                  menu.colors.normal.bg,
                                  menu.colors.normal.fg,
                                  True, False, False);

          barwin_map(item[i]);
          barwin_refresh_color(item[i]);
          menu_draw_item_name(&menu, i, item);
          barwin_refresh(item[i]);
     }

     /* Select the first item */
     menu_focus_item(&menu, 0, item);

     XGrabKeyboard(dpy, ROOT, True, GrabModeAsync, GrabModeAsync, CurrentTime);

     while(!menu_manage_event(&ev, &menu, item));

     XUngrabKeyboard(dpy, CurrentTime);

     for(i = 0; i < menu.nitem; ++i)
          barwin_delete(item[i]);
     barwin_delete(frame);

     return;
}

Bool
menu_manage_event(XEvent *ev, Menu *menu, BarWindow *winitem[])
{
     int i;
     KeySym ks;
     Bool quit = False;

     switch(ev->type)
     {
          /* Mouse Buttons */
     case ButtonPress:
          /* Execute the function linked with the item */
          for(i = 0; i < menu->nitem; ++i)
               if(ev->xbutton.window == winitem[i]->win
                  && (ev->xbutton.button == Button1 || ev->xbutton.button == Button2))
                    if(menu->item[i].func)
                         menu->item[i].func(menu->item[i].cmd);
          quit = True;
          break;

          /* Keys */
     case KeyPress:
          XLookupString(&ev->xkey, NULL, 0, &ks, 0);
          switch(ks)
          {
          case XK_Up:
               menu_focus_item(menu, menu->focus_item - 1, winitem);
               break;
          case XK_Down:
               menu_focus_item(menu, menu->focus_item + 1, winitem);
               break;
          case XK_Return:
               if(menu->item[menu->focus_item].func)
                    menu->item[menu->focus_item].func(menu->item[menu->focus_item].cmd);
               quit = True;
               break;
          case XK_Escape:
               quit = True;
               break;
          }
          break;

          /* Focus (with mouse) management */
     case EnterNotify:
          /* For focus an item with the mouse */
          for(i = 0; i < menu->nitem; ++i)
               if(ev->xcrossing.window == winitem[i]->win)
                    menu_focus_item(menu, i, winitem);
          break;
     default:
          getevent(*ev);
          break;
     }
     XNextEvent(dpy, ev);

     return quit;
}


void
menu_focus_item(Menu *menu, int item, BarWindow *winitem[])
{
     int i;

     menu->focus_item = item;

     if(menu->focus_item > menu->nitem - 1)
          menu->focus_item = 0;
     else if(menu->focus_item < 0)
          menu->focus_item = menu->nitem - 1;

     for(i = 0; i < menu->nitem; ++i)
     {
          if(i == menu->focus_item)
          {
               winitem[i]->fg = menu->colors.focus.fg;
               winitem[i]->bg = menu->colors.focus.bg;
          }
          else
          {
               winitem[i]->fg = menu->colors.normal.fg;
               winitem[i]->bg = menu->colors.normal.bg;
          }

          barwin_refresh_color(winitem[i]);
          menu_draw_item_name(menu, i, winitem);
          barwin_refresh(winitem[i]);
     }

     return;
}

void
menu_draw_item_name(Menu *menu, int item, BarWindow *winitem[])
{
     int width = menu_get_longer_string(menu->item, menu->nitem);

     barwin_draw_text(winitem[item],
                      ((width / 2) - (textw(menu->item[item].name) / 2)),
                      font->height,
                      menu->item[item].name);

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

void
uicb_menu(uicb_t cmd)
{
     int i, d, u, x, y;
     Window w;

     for(i = 0; i < conf.nmenu; ++i)
          if(!strcmp(cmd, conf.menu[i].name))
          {
               if(conf.menu[i].place_at_mouse)
                    XQueryPointer(dpy, ROOT, &w, &w, &x, &y, &d, &d, (uint *)&u);
               else
               {
                    screen_get_sel();
                    x = sgeo[selscreen].x + conf.menu[i].x;
                    y = sgeo[selscreen].y + conf.menu[i].y;
               }
               menu_draw(conf.menu[i], x, y);
          }

     return;
}
