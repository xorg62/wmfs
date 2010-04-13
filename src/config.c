/*
*      config.c
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

func_name_list_t tmp_func_list[] =
{
     {"spawn",                    uicb_spawn },
     {"client_kill",              uicb_client_kill },
     {"client_prev",              uicb_client_prev },
     {"client_next",              uicb_client_next },
     {"client_swap_next",         uicb_client_swap_next },
     {"client_swap_prev",         uicb_client_swap_prev },
     {"client_screen_next",       uicb_client_screen_next },
     {"client_screen_prev",       uicb_client_screen_prev },
     {"client_move",              uicb_client_move },
     {"client_resize",            uicb_client_resize },
     {"toggle_max",               uicb_togglemax },
     {"layout_next",              uicb_layout_next },
     {"layout_prev",              uicb_layout_prev },
     {"tag",                      uicb_tag },
     {"tag_next",                 uicb_tag_next },
     {"tag_prev",                 uicb_tag_prev },
     {"tag_prev_sel",             uicb_tag_prev_sel },
     {"tag_transfert",            uicb_tagtransfert },
     {"tag_transfert_next",       uicb_tagtransfert_next },
     {"tag_transfert_prev",       uicb_tagtransfert_prev },
     {"tag_urgent",               uicb_tag_urgent },
     {"tag_toggle_additional",    uicb_tag_toggle_additional },
     {"set_mwfact",               uicb_set_mwfact },
     {"set_nmaster",              uicb_set_nmaster },
     {"quit",                     uicb_quit },
     {"toggle_infobar_position",  uicb_infobar_togglepos },
     {"toggle_resizehint",        uicb_toggle_resizehint },
     {"mouse_move",               uicb_mouse_move },
     {"mouse_resize",             uicb_mouse_resize },
     {"client_raise",             uicb_client_raise },
     {"toggle_free",              uicb_togglefree },
     {"toggle_abovefc",           uicb_toggle_abovefc },
     {"screen_select",            uicb_screen_select },
     {"screen_next",              uicb_screen_next },
     {"screen_prev",              uicb_screen_prev },
     {"screen_prev_sel",          uicb_screen_prev_sel},
     {"reload",                   uicb_reload },
     {"launcher",                 uicb_launcher },
     {"set_layout",               uicb_set_layout },
     {"menu",                     uicb_menu },
     {"set_client_layer",         uicb_set_client_layer },
     {"set_layer",                uicb_set_layer },
     {"ignore_next_client_rules", uicb_ignore_next_client_rules },
     {"check_max",                uicb_checkmax },
     {"check_free",               uicb_checkfree },
     {"check_layout",             uicb_checklayout }
};

key_name_list_t key_list[] =
{
     {"Control", ControlMask },
     {"Shift",   ShiftMask },
     {"Lock",    LockMask },
     {"Alt",     Mod1Mask },
     {"Mod2",    Mod2Mask },
     {"Mod3",    Mod3Mask },
     {"Mod4",    Mod4Mask },
     {"Super",   Mod4Mask },
     {"Home",    Mod4Mask },
     {"Mod5",    Mod5Mask },
     {NULL,      NoSymbol }
};

name_to_uint_t mouse_button_list[] =
{
     {"Button1", Button1 },
     {"Button2", Button2 },
     {"Button3", Button3 },
     {"Button4", Button4 },
     {"Button5", Button5 },
     {"1", Button1 },
     {"2", Button2 },
     {"3", Button3 },
     {"4", Button4 },
     {"5", Button5 },
};

void
mouse_section(MouseBinding mb[], int ns, struct conf_sec **sec)
{
     int n;

     for (n = 0; sec[n]; n++)
     {
          mb[n].tag    = fetch_opt(sec[n], "-1", "tag")[0].num;
          mb[n].screen = fetch_opt(sec[n], "-1", "screen")[0].num;
          mb[n].button = char_to_button(fetch_opt(sec[n], "1", "button")[0].str, mouse_button_list);
          mb[n].func   = name_to_func(fetch_opt(sec[n], "", "func")[0].str, func_list);
          mb[n].cmd    = fetch_opt(sec[n], "", "cmd")[0].str;
     }
}

void
conf_misc_section(void)
{
     int pad = 12;
     struct conf_sec **sec;

     sec = fetch_section(NULL, "misc");

     conf.font              = fetch_opt(sec[0], "sans-9", "font")[0].str;
     conf.raisefocus        = fetch_opt(sec[0], "false", "raisefocus")[0].bool;
     conf.raiseswitch       = fetch_opt(sec[0], "false", "raiseswitch")[0].bool;
     conf.focus_fmouse      = fetch_opt(sec[0], "true", "focus_follow_mouse")[0].bool;
     conf.focus_pclick      = fetch_opt(sec[0], "true", "focus_pointer_click")[0].bool;
     conf.status_timing     = fetch_opt(sec[0], "1", "status_timing")[0].num;
     conf.status_path       = fetch_opt(sec[0], "", "status_path")[0].str;
     conf.autostart_path    = fetch_opt(sec[0], "", "autostart_path")[0].str;
     conf.autostart_command = fetch_opt(sec[0], "", "autostart_command")[0].str;
     pad                    = fetch_opt(sec[0], "12", "pad")[0].num;

     if(pad > 24 || pad < 1)
     {
          warnx("configuration : pad value (%d) incorrect.", pad);

          pad = 12;
     }

     conf.pad = pad;

     if(conf.status_timing <= 0)
     {
          warnx("configuration : status_timing value (%d) incorrect.", conf.status_timing);
          conf.status_timing = 1;
     }

     free(sec);

     return;
}

void
conf_bar_section(void)
{
     struct conf_sec **sec, **mouse;
     int n;

     sec = fetch_section(NULL, "bar");

     conf.border.bar  = fetch_opt(sec[0], "false", "border")[0].bool;
     conf.bars.height = fetch_opt(sec[0], "-1", "height")[0].num;
     conf.colors.bar  = getcolor(fetch_opt(sec[0], "#000000", "bg")[0].str);
     conf.colors.text = fetch_opt(sec[0], "#ffffff", "fg")[0].str;
     conf.bars.selbar = fetch_opt(sec[0], "false", "selbar")[0].bool;

     mouse = fetch_section(sec[0], "mouse");

     if (!mouse)
          return;

     for (n = 0; mouse[n] ; n++);

     conf.bars.nmouse = n;

     if (n > 0)
     {
          conf.bars.mouse = emalloc(n, sizeof(MouseBinding));
          mouse_section(conf.bars.mouse, n, mouse);
     }

     free(sec);
     free(mouse);

     return;
}

void
conf_root_section(void)
{
     struct conf_sec **sec, **mouse;
     int n;

     sec = fetch_section(NULL, "root");

     conf.root.background_command = fetch_opt(sec[0], "", "background_command")[0].str;

     mouse = fetch_section(sec[0], "mouse");

     if (mouse)
     {
        for(n = 0; mouse[n]; n++);
        if ((conf.root.nmouse = n) > 0) {
             conf.root.mouse = emalloc(n, sizeof(MouseBinding));
             mouse_section(conf.root.mouse, n, mouse);
        }
        free(mouse);
     }
     free(sec);

     return;
}

void
conf_client_section(void)
{
     int i, j, n;
     char *flags, *p;
     struct conf_sec **sec, **mouse, **titlebar, **button, **line;
     struct opt_type *opt;

     sec = fetch_section(NULL, "client");

     conf.client_round               = fetch_opt(sec[0], "true", "client_round")[0].bool;

     if ((conf.client.borderheight = fetch_opt(sec[0], "1", "border_height")[0].num) < 1)
          conf.client.borderheight = 1;

     conf.client.border_shadow       = fetch_opt(sec[0], "false", "border_shadow")[0].bool;
     conf.client.place_at_mouse      = fetch_opt(sec[0], "false", "place_at_mouse")[0].bool;
     conf.client.bordernormal        = getcolor(fetch_opt(sec[0], "#000000", "border_normal")[0].str);
     conf.client.borderfocus         = getcolor(fetch_opt(sec[0], "#ffffff", "border_focus")[0].str);
     conf.client.resizecorner_normal = getcolor(fetch_opt(sec[0], "#222222", "resize_corner_normal")[0].str);
     conf.client.resizecorner_focus  = getcolor(fetch_opt(sec[0], "#DDDDDD", "resize_corner_focus")[0].str);
     conf.client.mod                 |= char_to_modkey(fetch_opt(sec[0], "Alt", "modifier")[0].str, key_list);
     conf.client.set_new_win_master  = fetch_opt(sec[0], "true", "set_new_win_master")[0].bool;

     mouse = fetch_section(sec[0], "mouse");

     for(n = 0; mouse[n]; n++);

     if((conf.client.nmouse = n) > 0)
     {
          conf.client.mouse = emalloc(conf.client.nmouse, sizeof(MouseBinding));
          mouse_section(conf.client.mouse, n, mouse);
     }
     free(mouse);

     titlebar = fetch_section(sec[0], "titlebar");

     conf.titlebar.height    = fetch_opt(titlebar[0], "0", "height")[0].num;
     conf.titlebar.fg_normal = fetch_opt(titlebar[0], "#ffffff", "fg_normal")[0].str;
     conf.titlebar.fg_focus  = fetch_opt(titlebar[0], "#000000", "fg_focus")[0].str;

     conf.titlebar.stipple.active = fetch_opt(titlebar[0], "false", "stipple")[0].bool;

     if(!strcmp((p = fetch_opt(titlebar[0], "-1", "stipple_normal")[0].str), "-1"))
          conf.titlebar.stipple.colors.normal = getcolor(conf.titlebar.fg_normal);
     else
          conf.titlebar.stipple.colors.normal = getcolor(p);

     if(!strcmp((p = fetch_opt(titlebar[0], "-1", "stipple_focus")[0].str), "-1"))
          conf.titlebar.stipple.colors.focus = getcolor(conf.titlebar.fg_focus);
     else
          conf.titlebar.stipple.colors.focus = getcolor(p);

     mouse = fetch_section(titlebar[0], "mouse");

     for(n = 0; mouse[n]; n++);

     if((conf.titlebar.nmouse = n) > 0)
     {
          conf.titlebar.mouse = emalloc(conf.titlebar.nmouse, sizeof(MouseBinding));
          mouse_section(conf.titlebar.mouse, n, mouse);
     }

     free(mouse);

     /* Multi button part */
     button = fetch_section(titlebar[0], "button");

     for(n = 0; button[n]; n++);

     if((conf.titlebar.nbutton = n) > 0)
     {
          conf.titlebar.button = emalloc(conf.titlebar.nbutton, sizeof(Button));
          for(i = 0; i < conf.titlebar.nbutton; ++i)
          {
               flags = fetch_opt(button[n], "none", "flags")[0].str;

               conf.titlebar.button[i].flags = 0;
               if(strstr(flags, "free"))
                    conf.titlebar.button[i].flags |= FreeFlag;
               if(strstr(flags, "max"))
                    conf.titlebar.button[i].flags |= MaxFlag;
               if(strstr(flags, "tile"))
                    conf.titlebar.button[i].flags |= TileFlag;

               /* Multi mouse section */
               mouse = fetch_section(button[n], "mouse");

               for(n = 0; mouse[n]; n++);

               if((conf.titlebar.button[i].nmouse = n) > 0)
               {
                    conf.titlebar.button[i].mouse = emalloc(conf.titlebar.button[i].nmouse, sizeof(MouseBinding));
                    mouse_section(conf.titlebar.button[i].mouse, n, mouse);
               }
               free(mouse);

               /* Multi line section */
               line = fetch_section(button[n], "line");

               for (n = 0; line[n]; n++);

               if((conf.titlebar.button[i].nlines = n) > 0)
               {
                    conf.titlebar.button[i].linecoord = emalloc(conf.titlebar.button[i].nlines, sizeof(XSegment));

                    for(j = 0; j < n; ++j)
                    {
                         opt = fetch_opt(line[j], "0", "coord");
                         conf.titlebar.button[i].linecoord[j].x1 = opt[0].num;
                         conf.titlebar.button[i].linecoord[j].y1 = opt[1].num;
                         conf.titlebar.button[i].linecoord[j].x2 = opt[2].num;
                         conf.titlebar.button[i].linecoord[j].y2 = opt[3].num;
                    }
               }
               free(line);
          }
     }
     free(button);
     free(titlebar);
     free(sec);

     return;
}

void
conf_layout_section(void)
{
     int i;
     char *tmp = NULL, *p;
     struct conf_sec **layouts, **layout;
     int n;

     /* Set conf.layout NULL for conf reload */
     for(i = 0; i < NUM_OF_LAYOUT; ++i)
     {
          conf.layout[i].symbol = NULL;
          conf.layout[i].func = NULL;
     }

     layouts = fetch_section(NULL, "layouts");

     conf.border.layout     = fetch_opt(layouts[0], "false", "border")[0].bool;
     conf.colors.layout_fg  = fetch_opt(layouts[0], "#ffffff", "fg")[0].str;
     conf.colors.layout_bg  = getcolor((fetch_opt(layouts[0], "#000000", "bg")[0].str));


     if((tmp = fetch_opt(layouts[0], "menu", "system")[0].str) && !strcmp(tmp, "menu"))
          conf.layout_system = True;

     if((tmp = fetch_opt(layouts[0], "right", "placement")[0].str) && !strcmp(tmp, "left"))
          conf.layout_placement = True;

     layout = fetch_section(layouts[0], "layout");

     for (n = 0; layout[n]; n++);

     conf.nlayout = n;

     if(conf.nlayout > NUM_OF_LAYOUT || !(conf.nlayout))
     {
          warnx("configuration : Too many or no layouts (%d).", conf.nlayout);
          conf.nlayout          = 1;
          conf.layout[0].symbol = _strdup("TILE");
          conf.layout[0].func   = tile;
     }

     if(conf.layout_system && conf.nlayout > 1)
     {
          menu_init(&menulayout, "menulayout", conf.nlayout,
                    /* Colors */
                    conf.colors.layout_bg,
                    conf.colors.layout_fg,
                    conf.colors.bar,
                    conf.colors.text);
     }

     if(!conf.layout[0].symbol
               && !conf.layout[0].func)
     {
          for(i = 0; i < conf.nlayout; ++i)
          {
               if(!name_to_func((p = fetch_opt(layout[i], "tile", "type")[0].str), layout_list))
                    warnx("configuration : Unknown Layout type : \"%s\".", p);
               else
               {
                    if(conf.layout_system && conf.nlayout > 1)
                    {
                         menu_new_item(&menulayout.item[i], fetch_opt(layout[i], "", "symbol")[0].str,
                                   uicb_set_layout, p);

                         menulayout.item[i].check = name_to_func("check_layout", func_list);
                    }

                    conf.layout[i].symbol = fetch_opt(layout[i], "TILE (default)", "symbol")[0].str;
                    conf.layout[i].func = name_to_func(p, layout_list);
                    conf.layout[i].type = p;
               }
          }
     }
     free(layout);
     free(layouts);

     return;
}

void
conf_tag_section(void)
{
     int i, j, k, l = 0, m, n, sc, count;
     char *tmp;
     struct conf_sec **sec, **tag, **mouse;
     struct opt_type *opt;

     /* If there is no tag in the conf or more than
      * MAXTAG (32) print an error and create only one.
      */
     Tag default_tag = { "WMFS", NULL, 0, 1,
                         0.50, 1, False, False, False, False, IB_Top,
                         layout_name_to_struct(conf.layout, "tile_right", conf.nlayout, layout_list),
                         0, NULL, 0 };

     sec = fetch_section(NULL, "tags");

     conf.tag_round               = fetch_opt(sec[0], "false", "tag_round")[0].bool;
     conf.colors.tagselfg         = fetch_opt(sec[0], "#ffffff", "sel_fg")[0].str;
     conf.colors.tagselbg         = getcolor(fetch_opt(sec[0], "#000000", "sel_bg")[0].str);
     conf.colors.tagurfg          = fetch_opt(sec[0], "#000000", "urgent_fg")[0].str;
     conf.colors.tagurbg          = getcolor(fetch_opt(sec[0], "#DD1111", "urgent_bg")[0].str);
     conf.colors.tag_occupied_bg  = getcolor(fetch_opt(sec[0], "#222222", "occupied_bg")[0].str);
     conf.border.tag              = fetch_opt(sec[0], "false", "border")[0].bool;

     /* Mouse button action on tag */
     conf.mouse_tag_action[TagSel] =
          char_to_button(fetch_opt(sec[0], "1", "mouse_button_tag_sel")[0].str, mouse_button_list);
     conf.mouse_tag_action[TagTransfert] =
          char_to_button(fetch_opt(sec[0], "2", "mouse_button_tag_transfert")[0].str, mouse_button_list);
     conf.mouse_tag_action[TagAdd] =
          char_to_button(fetch_opt(sec[0], "3", "mouse_button_tag_add")[0].str, mouse_button_list);
     conf.mouse_tag_action[TagNext]  =
          char_to_button(fetch_opt(sec[0], "4", "mouse_button_tag_next")[0].str, mouse_button_list);
     conf.mouse_tag_action[TagPrev]  =
          char_to_button(fetch_opt(sec[0], "5", "mouse_button_tag_prev")[0].str, mouse_button_list);

     sc = screen_count();

     /* Alloc all */
     conf.ntag  = emalloc(sc, sizeof(int));
     tags       = emalloc(sc, sizeof(Tag*));
     seltag     = emalloc(sc, sizeof(int));
     prevseltag = emalloc(sc, sizeof(int));

     for(i = 0; i < sc; ++i)
          seltag[i] = 1;

     tag = fetch_section(sec[0], "tag");

     for(n = 0; tag[n]; n++);

     for(i = 0; i < sc; ++i)
          tags[i] = emalloc(n + 2, sizeof(Tag));

     for(i = (n - 1); i >= 0; i--)
     {
          j = fetch_opt(tag[i], "-1", "screen")[0].num;

          if(j < 0 || j > sc - 1)
               j = -1;

          for(k = ((j == -1) ? 0 : j);
              ((j == -1) ? (k < sc) : !l);
              ((j == -1) ? ++k : --l))
          {
               ++conf.ntag[k];
               tags[k][conf.ntag[k]].name       = fetch_opt(tag[i], "", "name")[0].str;
               tags[k][conf.ntag[k]].mwfact     = fetch_opt(tag[i], "0.65", "mwfact")[0].fnum;
               tags[k][conf.ntag[k]].nmaster    = fetch_opt(tag[i], "1", "nmaster")[0].num;
               tags[k][conf.ntag[k]].resizehint = fetch_opt(tag[i], "false", "resizehint")[0].bool;
               tags[k][conf.ntag[k]].abovefc    = fetch_opt(tag[i], "false", "abovefc")[0].bool;
               tags[k][conf.ntag[k]].layers = 1;

               tmp = fetch_opt(tag[i], "top", "infobar_position")[0].str;

               if(!strcmp(tmp ,"none") || !strcmp(tmp, "hide") || !strcmp(tmp, "hidden"))
                    tags[k][conf.ntag[k]].barpos = IB_Hide;
               else if(!strcmp(tmp, "bottom") || !strcmp(tmp, "down"))
                    tags[k][conf.ntag[k]].barpos = IB_Bottom;
               else
                    tags[k][conf.ntag[k]].barpos = IB_Top;

               tags[k][conf.ntag[k]].layout = layout_name_to_struct(conf.layout,
                                                                    fetch_opt(tag[i], "tile_right", "layout")[0].str,
                                                                    conf.nlayout,
                                                                    layout_list);

               /* Clients list */
               opt = fetch_opt(tag[i], "", "clients");

               for (count = 0; opt[count].str; count++);

               if(count)
               {
                    tags[k][conf.ntag[k]].nclients = count;
                    tags[k][conf.ntag[k]].clients = emalloc(count, sizeof(char *));
                    for(m = 0; m < count; ++m)
                         tags[k][conf.ntag[k]].clients[m] = opt[m].str;
               }

               /* Multi mouse sections */
               mouse = fetch_section(tag[i], "mouse");

               for (count = 0; mouse[count]; count++);

               if((tags[k][conf.ntag[k]].nmouse = count))
               {
                    tags[k][conf.ntag[k]].mouse = emalloc(tags[k][conf.ntag[k]].nmouse, sizeof(MouseBinding));
                    mouse_section(tags[k][conf.ntag[k]].mouse, count, mouse);
               }
               free(mouse);
          }
          l = 0;
     }

     for(i = 0; i < sc; ++i)
          if(!conf.ntag[i] || conf.ntag[i] > MAXTAG)
          {
               warnx("configuration : Too many or no tag (%d) in the screen %d.", conf.ntag[i], i);
               conf.ntag[i] = 1;
               tags[i][1] = default_tag;
          }

     free(tag);
     free(sec);

     return;
}

void
conf_menu_section(void)
{
     char *tmp2;
     int i, j, aj,  n;
     struct conf_sec **menu, **set_menu, **item;

     menu = fetch_section(NULL, "menu");
     set_menu = fetch_section(menu[0], "set_menu");

     for (n = 0; set_menu[n]; n++);


     CHECK((conf.nmenu = n));

     conf.menu = calloc(conf.nmenu, sizeof(Menu));

     for(i = 0; i < conf.nmenu; ++i)
     {

          conf.menu[i].name = fetch_opt(set_menu[i], "menu_wname", "name")[0].str;

          if(!(conf.menu[i].place_at_mouse = fetch_opt(set_menu[i], "true", "place_at_mouse")[0].bool))
          {
               conf.menu[i].x = fetch_opt(set_menu[i], "0", "x")[0].num;
               conf.menu[i].y = fetch_opt(set_menu[i], "0", "y")[0].num;
          }

          tmp2 = fetch_opt(set_menu[i], "center", "align")[0].str;

          if(!strcmp(tmp2 ,"left"))
               conf.menu[i].align = MA_Left;
          else if(!strcmp(tmp2, "right"))
               conf.menu[i].align = MA_Right;
          else
               conf.menu[i].align = MA_Center;

          conf.menu[i].colors.focus.bg  = getcolor(fetch_opt(set_menu[i], "#000000", "bg_focus")[0].str);
          conf.menu[i].colors.focus.fg  = fetch_opt(set_menu[i], "#ffffff", "fg_focus")[0].str;
          conf.menu[i].colors.normal.bg = getcolor(fetch_opt(set_menu[i], "#000000", "bg_normal")[0].str);
          conf.menu[i].colors.normal.fg = fetch_opt(set_menu[i], "#ffffff", "fg_normal")[0].str;

          item = fetch_section(set_menu[i], "item");

          for (n = 0; item[n]; n++);

          if((conf.menu[i].nitem = n))
          {
               conf.menu[i].item = emalloc(conf.menu[i].nitem, sizeof(MenuItem));
               for(j = 0; j < n; ++j)
               {
                    aj = (n - 1) - j;
                    conf.menu[i].item[aj].name = fetch_opt(item[j], "item_wname", "name")[0].str;
                    conf.menu[i].item[aj].func = name_to_func(fetch_opt(item[j], "", "func")[0].str, func_list);
                    conf.menu[i].item[aj].cmd  = fetch_opt(item[j], "", "cmd")[0].str;
                    conf.menu[i].item[aj].check = name_to_func(fetch_opt(item[j], "", "check")[0].str, func_list);
                    conf.menu[i].item[aj].submenu = fetch_opt(item[j], "", "submenu")[0].str;
               }
          }
          free(item);
     }
     free(set_menu);
     free(menu);

     return;
}

void
conf_launcher_section(void)
{
     int i, n;
     struct conf_sec **launcher, **set_launcher;

     launcher = fetch_section(NULL, "launcher");
     set_launcher = fetch_section(launcher[0], "set_launcher");

     for (n = 0; set_launcher[n]; n++);

     CHECK((conf.nlauncher = n));

     conf.launcher = emalloc(conf.nlauncher, sizeof(Launcher));

     for(i = 0; i < conf.nlauncher; ++i)
     {
          conf.launcher[i].name    = fetch_opt(set_launcher[i], "launcher", "name")[0].str;
          conf.launcher[i].prompt  = fetch_opt(set_launcher[i], "Exec:", "prompt")[0].str;
          conf.launcher[i].command = fetch_opt(set_launcher[i], "exec", "command")[0].str;
          conf.launcher[i].nhisto  = 1;
     }
     free(set_launcher);
     free(launcher);

     return;
}

void
conf_keybind_section(void)
{
     int i, j, n = 0;
     struct conf_sec **sec, **ks;
     struct opt_type *opt;

     sec = fetch_section(NULL, "keys");
     ks = fetch_section(sec[0], "key");

     for (n = 0; ks[n]; n++);

     conf.nkeybind = n;
     keys = emalloc(conf.nkeybind, sizeof(Key));

     for(i = 0; i < conf.nkeybind; ++i)
     {
          opt = fetch_opt(ks[i], "", "mod");

          for (n = 0; opt[n].str; n++);

          for(j = 0; j < n; ++j)
               keys[i].mod |= char_to_modkey(opt[j].str, key_list);

          keys[i].keysym = XStringToKeysym(fetch_opt(ks[i], "None", "key")[0].str);

          keys[i].func = name_to_func(fetch_opt(ks[i], "", "func")[0].str, func_list);

          if(keys[i].func == NULL)
          {
               warnx("configuration : Unknown Function \"%s\".", fetch_opt(ks[i], "", "func")[0].str);
               keys[i].func = uicb_spawn;
          }

          keys[i].cmd = fetch_opt(ks[i], "", "cmd")[0].str;
     }

     return;
}

/** Configuration initialization
*/
void
init_conf(void)
{
     if (get_conf(conf.confpath) == -1)
     {
          warnx("parsing configuration file (%s) failed.", conf.confpath);
          sprintf(conf.confpath, "%s/wmfs/wmfsrc", XDG_CONFIG_DIR);
          warnx("Use the default configuration (%s).", conf.confpath);
          if (get_conf(conf.confpath) == -1)
               errx(1, "parsing configuration file (%s) failed.", conf.confpath);

     }

     /* Set func_list */
     func_list = emalloc(LEN(tmp_func_list), sizeof(func_name_list_t));
     memcpy(func_list, tmp_func_list, LEN(tmp_func_list) * sizeof(func_name_list_t));

     conf_misc_section();
     conf_bar_section();
     conf_root_section();
     conf_client_section();
     conf_layout_section();
     conf_tag_section();
     conf_menu_section();
     conf_launcher_section();
     conf_keybind_section();

     return;
}


