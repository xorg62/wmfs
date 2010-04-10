/*
*      menu.c
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
     int i, width, height, out;
     XEvent ev;
     BarWindow *item[menu.nitem];
     BarWindow *frame;

     width = menu_get_longer_string(menu.item, menu.nitem) + PAD * 3;
     height = menu.nitem * (INFOBARH - SHADH);

     /* Frame barwin */
     screen_get_sel();

     if((out = x + width  - MAXW)  > 0)
          x -= out;
     if((out = y + height - MAXH) > 0)
          y -= out;

     frame = barwin_create(ROOT, x, y, width + SHADH, height + SHADH * 2,
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
     int i, c = 0;
     KeySym ks;
     Bool quit = False;

     switch(ev->type)
     {
     /* Mouse events */
     case ButtonPress:
          /* Execute the function linked with the item */
          for(i = 0; i < menu->nitem; ++i)
          {
               if(ev->xbutton.window == winitem[i]->win
                  && (ev->xbutton.button == Button1 || ev->xbutton.button == Button2))
                    quit = menu_activate_item(menu, i);
               else if(ev->xbutton.window != winitem[i]->win)
                    ++c;
               else if(ev->xbutton.button == Button4)
                    menu_focus_item(menu, menu->focus_item - 1, winitem);
               else if(ev->xbutton.button == Button5)
                    menu_focus_item(menu, menu->focus_item + 1, winitem);
          }

          /* If the clicked window is not one of menu wins (items), quit. */
          if(c == i)
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
               quit = menu_activate_item(menu, menu->focus_item);
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

Bool
menu_activate_item(Menu *menu, int i)
{
     int j, x, y;

     if(menu->item[i].submenu)
     {
          for(j = 0; j < conf.nmenu; ++j)
               if(!strcmp(menu->item[i].submenu, conf.menu[j].name))
               {
                    y = menu->y + ((i - 1) * INFOBARH + PAD) - SHADH * 2;
                    x = menu->x + menu_get_longer_string(menu->item, menu->nitem) + PAD * 3;

                    menu_draw(conf.menu[j], x, y);

                    return True;
               }
     }
     else if(menu->item[i].func)
     {
          menu->item[i].func(menu->item[i].cmd);

          return True;
     }

     return False;
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
          winitem[i]->fg = ((i == menu->focus_item) ? menu->colors.focus.fg : menu->colors.normal.fg);
          winitem[i]->bg = ((i == menu->focus_item) ? menu->colors.focus.bg : menu->colors.normal.bg);

          barwin_refresh_color(winitem[i]);
          menu_draw_item_name(menu, i, winitem);
          barwin_refresh(winitem[i]);
     }

     return;
}

void
menu_draw_item_name(Menu *menu, int item, BarWindow *winitem[])
{
     int x;
     int width = menu_get_longer_string(menu->item, menu->nitem);

     switch(menu->align)
     {
     case MA_Left:
          x = PAD * 3 / 2;
          break;
     case MA_Right:
          x = width - textw(menu->item[item].name) + PAD * 3 / 2;
          break;
     default:
     case MA_Center:
          x = width / 2 - textw(menu->item[item].name) / 2 + PAD * 3 / 2;
          break;
     }
     barwin_draw_text(winitem[item], x, FHINFOBAR, menu->item[item].name);

     if(menu->item[item].check)
          if(menu->item[item].check(menu->item[item].cmd))
               barwin_draw_text(winitem[item], PAD / 3, FHINFOBAR, "*");

     if(menu->item[item].submenu)
          barwin_draw_text(winitem[item], width + PAD * 2, FHINFOBAR, ">");

     return;
}

int
menu_get_longer_string(MenuItem *mi, int nitem)
{
     int i, w, l = 0;

     for(i = 0; i < nitem; ++i)
          if((w = textw(mi[i].name)) > l)
               l = w;

     return l;
}

void
uicb_menu(uicb_t cmd)
{
     int i, d, u, x, y;
     Window w;

     if(!strcmp(cmd, "menulayout"))
          menu_draw(menulayout, menulayout.x, menulayout.y);

     for(i = 0; i < conf.nmenu; ++i)
          if(!strcmp(cmd, conf.menu[i].name))
          {
               if(conf.menu[i].place_at_mouse)
               {
                    XQueryPointer(dpy, ROOT, &w, &w, &x, &y, &d, &d, (uint *)&u);
                    conf.menu[i].x = x;
                    conf.menu[i].y = y;
               }
               else
               {
                    screen_get_sel();
                    x = conf.menu[i].x + spgeo[selscreen].x;
                    y = conf.menu[i].y + spgeo[selscreen].y;
               }
               menu_draw(conf.menu[i], x, y);
          }

     return;
}
