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

const func_name_list_t func_list[] =
{
     {"spawn",                    uicb_spawn },
     {"client_kill",              uicb_client_kill },
     {"client_prev",              uicb_client_prev },
     {"client_next",              uicb_client_next },
     {"client_swap_next",         uicb_client_swap_next },
     {"client_swap_prev",         uicb_client_swap_prev },
     {"client_screen_next",       uicb_client_screen_next },
     {"client_screen_prev",       uicb_client_screen_prev },
     {"client_screen_set",        uicb_client_screen_set },
     {"client_focus_right",       uicb_client_focus_right },
     {"client_focus_left" ,       uicb_client_focus_left },
     {"client_focus_top",         uicb_client_focus_top },
     {"client_focus_bottom",      uicb_client_focus_bottom },
     {"client_move",              uicb_client_move },
     {"client_resize",            uicb_client_resize },
     {"client_ignore_tag",        uicb_client_ignore_tag },
     {"client_set_master",        uicb_client_set_master },
     {"client_resize_right",      uicb_client_resize_right },
     {"client_resize_left",       uicb_client_resize_left },
     {"client_resize_top",        uicb_client_resize_top },
     {"client_resize_bottom",     uicb_client_resize_bottom },
     {"toggle_max",               uicb_togglemax },
     {"layout_next",              uicb_layout_next },
     {"layout_prev",              uicb_layout_prev },
     {"tag",                      uicb_tag },
     {"tag_next",                 uicb_tag_next },
     {"tag_prev",                 uicb_tag_prev },
     {"tag_next_visible",         uicb_tag_next_visible },
     {"tag_prev_visible",         uicb_tag_prev_visible },
     {"tag_prev_sel",             uicb_tag_prev_sel },
     {"tag_transfert",            uicb_tagtransfert },
     {"tag_transfert_next",       uicb_tagtransfert_next },
     {"tag_transfert_prev",       uicb_tagtransfert_prev },
     {"tag_urgent",               uicb_tag_urgent },
     {"tag_toggle_additional",    uicb_tag_toggle_additional },
     {"tag_swap",                 uicb_tag_swap },
     {"tag_swap_next",            uicb_tag_swap_next },
     {"tag_swap_prev",            uicb_tag_swap_previous },
     {"tag_new",                  uicb_tag_new },
     {"tag_del",                  uicb_tag_del },
     {"tag_rename",               uicb_tag_rename },
     {"tag_last",                 uicb_tag_last },
     {"tag_stay_last",            uicb_tag_stay_last },
     {"set_mwfact",               uicb_set_mwfact },
     {"set_nmaster",              uicb_set_nmaster },
     {"quit",                     uicb_quit },
     {"toggle_infobar_position",  uicb_infobar_togglepos },
     {"toggle_infobar_display",   uicb_infobar_toggledisplay },
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
     {"ignore_next_client_rules", uicb_ignore_next_client_rules },
     {"check_max",                uicb_checkmax },
     {"check_free",               uicb_checkfree },
     {"check_layout",             uicb_checklayout },
     {"clientlist",               uicb_clientlist },
     {"check_clist",              uicb_checkclist },
     {"toggle_tagautohide",       uicb_toggle_tagautohide },
     {"toggle_tag_expose",        uicb_tag_toggle_expose},
     /*{"split_client_vertical",    uicb_split_client_vertical },
     {"split_client_horizontal",  uicb_split_client_horizontal }, */
     {NULL, NULL}
};

static key_name_list_t key_list[] =
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

static name_to_uint_t mouse_button_list[] =
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
     {NULL, NoSymbol}
};

static void
mouse_section(MouseBinding mb[], struct conf_sec **sec)
{
     int n;

     for (n = 0; sec[n]; n++)
     {
          mb[n].tag    = fetch_opt_first(sec[n], "-1", "tag").num;
          mb[n].screen = fetch_opt_first(sec[n], "-1", "screen").num;
          mb[n].button = char_to_button(fetch_opt_first(sec[n], "1", "button").str, mouse_button_list);
          mb[n].func   = name_to_func(fetch_opt_first(sec[n], "", "func").str, func_list);
          mb[n].cmd    = fetch_opt_first(sec[n], "", "cmd").str;
     }
}

static void
conf_misc_section(void)
{
     int pad = 12;
     uint opacity = 255;
     struct conf_sec *sec;

     sec = fetch_section_first(NULL, "misc");

     conf.font              = fetch_opt_first(sec, "sans-9", "font").str;
     conf.raisefocus        = fetch_opt_first(sec, "false", "raisefocus").bool;
     conf.focus_fmouse      = fetch_opt_first(sec, "true", "focus_follow_mouse").bool;
     conf.focus_fmov        = fetch_opt_first(sec, "false", "focus_follow_movement").bool;
     conf.focus_pclick      = fetch_opt_first(sec, "true", "focus_pointer_click").bool;
     conf.status_timing     = fetch_opt_first(sec, "1", "status_timing").num;
     conf.status_path       = fetch_opt_first(sec, "", "status_path").str;
     conf.autostart_path    = fetch_opt_first(sec, "", "autostart_path").str;
     conf.autostart_command = fetch_opt_first(sec, "", "autostart_command").str;
     pad                    = fetch_opt_first(sec, "12", "pad").num;
     opacity                = fetch_opt_first(sec, "255", "opacity").num;

     if(opacity > 255)
          opacity = 255;

     conf.opacity = opacity << 24;

     if(pad > 24 || pad < 1)
     {
          warnx("configuration : pad value (%d) incorrect.", pad);

          pad = 12;
     }

     conf.pad = pad;

     if(conf.status_timing < 0)
     {
          warnx("configuration : status_timing value (%d) incorrect.", conf.status_timing);
          conf.status_timing = 1;
     }

     return;
}

static void
conf_bar_section(void)
{
     struct conf_sec *bar, **mouse, *selbar, *systray;
     char *barbg;
     int sc = screen_count();

     bar = fetch_section_first(NULL, "bar");

     conf.border.bar  = fetch_opt_first(bar, "false", "border").bool;
     conf.bars.height = fetch_opt_first(bar, "-1", "height").num;
     conf.colors.bar  = getcolor((barbg = fetch_opt_first(bar, "#000000", "bg").str));
     conf.colors.text = fetch_opt_first(bar, "#ffffff", "fg").str;

     conf.colors.bar_light_shade = fetch_opt_first(bar, "0.25", "light_shade").fnum;
     conf.colors.bar_dark_shade  = fetch_opt_first(bar, "-0.25", "dark_shade").fnum;

     mouse = fetch_section(bar, "mouse");

     if ((conf.bars.nmouse = fetch_section_count(mouse)) > 0)
     {
          conf.bars.mouse = xcalloc(conf.bars.nmouse, sizeof(MouseBinding));
          mouse_section(conf.bars.mouse, mouse);
     }

     free(mouse);

     if((systray = fetch_section_first(bar, "systray")))
     {
          conf.systray.active = fetch_opt_first(systray, "true", "active").bool;

          if((conf.systray.screen = fetch_opt_first(systray, "0", "screen").num) < 0
                    || conf.systray.screen >= sc)
               conf.systray.screen = 0;

          if((conf.systray.spacing = fetch_opt_first(systray, "3", "spacing").num) < 0)
               conf.systray.spacing = 0;
     }
     else
          conf.systray.active = False;

     selbar = fetch_section_first(bar, "selbar");
     conf.bars.selbar = selbar ? True : False;

     conf.selbar.bg = getcolor(fetch_opt_first(selbar, barbg, "bg").str);
     conf.selbar.fg = fetch_opt_first(selbar, conf.colors.text, "fg").str;
     conf.selbar.maxlength = fetch_opt_first(selbar, "-1", "max_length").num;

     mouse = fetch_section(selbar, "mouse");

     if ((conf.selbar.nmouse = fetch_section_count(mouse)))
     {
          conf.selbar.mouse = xcalloc(conf.selbar.nmouse, sizeof(MouseBinding));
          mouse_section(conf.selbar.mouse, mouse);
     }

     free(mouse);

     return;
}

static void
conf_root_section(void)
{
     struct conf_sec *root, **mouse;

     root = fetch_section_first(NULL, "root");

     conf.root.background_command = fetch_opt_first(root, "", "background_command").str;

     mouse = fetch_section(root, "mouse");

     if ((conf.root.nmouse = fetch_section_count(mouse)) > 0)
     {
          conf.root.mouse = xcalloc(conf.root.nmouse, sizeof(MouseBinding));
          mouse_section(conf.root.mouse, mouse);
     }

     free(mouse);

     return;
}

static void
conf_client_section(void)
{
     int i, j;
     char *flags, *p;
     struct conf_sec *sec, **mouse, *titlebar, **button, **line;
     struct opt_type *opt;

     sec = fetch_section_first(NULL, "client");

     conf.client_round               = fetch_opt_first(sec, "true", "client_round").bool;
     conf.client_auto_center         = fetch_opt_first(sec, "false", "client_auto_center").bool;
     conf.client_tile_raise          = fetch_opt_first(sec, "false", "client_tile_raise").bool;

     if ((conf.client.borderheight = fetch_opt_first(sec, "1", "border_height").num) < 1)
          conf.client.borderheight = 1;

     conf.client.border_shadow        = fetch_opt_first(sec, "false", "border_shadow").bool;
     conf.client.place_at_mouse       = fetch_opt_first(sec, "false", "place_at_mouse").bool;
     conf.client.bordernormal         = getcolor(fetch_opt_first(sec, "#000000", "border_normal").str);
     conf.client.borderfocus          = getcolor(fetch_opt_first(sec, "#ffffff", "border_focus").str);
     conf.client.resizecorner_normal  = getcolor(fetch_opt_first(sec, "#222222", "resize_corner_normal").str);
     conf.client.resizecorner_focus   = getcolor(fetch_opt_first(sec, "#DDDDDD", "resize_corner_focus").str);
     conf.client.mod                  |= char_to_modkey(fetch_opt_first(sec, "Alt", "modifier").str, key_list);
     conf.client.set_new_win_master   = fetch_opt_first(sec, "true", "set_new_win_master").bool;
     conf.client.padding              = fetch_opt_first(sec, "0", "padding").num;
     conf.client.autofree             = fetch_opt_first(sec, "", "autofree").str;
     conf.client.automax              = fetch_opt_first(sec, "", "automax").str;
     conf.client.default_open_tag     = fetch_opt_first(sec, "0", "default_open_tag").num;
     conf.client.default_open_screen  = fetch_opt_first(sec, "-1", "default_open_screen").num;
     conf.client.new_client_get_mouse = fetch_opt_first(sec, "false", "new_client_get_mouse").bool;

     conf.colors.client_light_shade = fetch_opt_first(sec, "0.25", "light_shade").fnum;
     conf.colors.client_dark_shade  = fetch_opt_first(sec, "-0.25", "dark_shade").fnum;

     mouse = fetch_section(sec, "mouse");

     if((conf.client.nmouse = fetch_section_count(mouse)) > 0)
     {
          conf.client.mouse = xcalloc(conf.client.nmouse, sizeof(MouseBinding));
          mouse_section(conf.client.mouse, mouse);
     }

     free(mouse);

     titlebar = fetch_section_first(sec, "titlebar");

     conf.titlebar.height    = fetch_opt_first(titlebar, "0", "height").num;
     conf.titlebar.fg_normal = fetch_opt_first(titlebar, "#ffffff", "fg_normal").str;
     conf.titlebar.fg_focus  = fetch_opt_first(titlebar, "#000000", "fg_focus").str;

     conf.titlebar.stipple.active = fetch_opt_first(titlebar, "false", "stipple").bool;

     if(!strcmp((p = fetch_opt_first(titlebar, "-1", "stipple_normal").str), "-1"))
          conf.titlebar.stipple.colors.normal = getcolor(conf.titlebar.fg_normal);
     else
          conf.titlebar.stipple.colors.normal = getcolor(p);

     if(!strcmp((p = fetch_opt_first(titlebar, "-1", "stipple_focus").str), "-1"))
          conf.titlebar.stipple.colors.focus = getcolor(conf.titlebar.fg_focus);
     else
          conf.titlebar.stipple.colors.focus = getcolor(p);

     mouse = fetch_section(titlebar, "mouse");

     if((conf.titlebar.nmouse = fetch_section_count(mouse)) > 0)
     {
          conf.titlebar.mouse = xcalloc(conf.titlebar.nmouse, sizeof(MouseBinding));
          mouse_section(conf.titlebar.mouse, mouse);
     }

     free(mouse);

     /* Multi button part */
     button = fetch_section(titlebar, "button");

     if((conf.titlebar.nbutton = fetch_section_count(button)) > 0)
     {
          conf.titlebar.button = xcalloc(conf.titlebar.nbutton, sizeof(Button));
          for(i = 0; i < conf.titlebar.nbutton; ++i)
          {
               flags = fetch_opt_first(button[i], "none", "flags").str;

               conf.titlebar.button[i].flags = 0;
               if(strstr(flags, "free"))
                    conf.titlebar.button[i].flags |= FreeFlag;
               if(strstr(flags, "max"))
                    conf.titlebar.button[i].flags |= MaxFlag;
               if(strstr(flags, "tile"))
                    conf.titlebar.button[i].flags |= TileFlag;

               /* Multi mouse section */
               mouse = fetch_section(button[i], "mouse");

               if((conf.titlebar.button[i].nmouse = fetch_section_count(mouse)) > 0)
               {
                    conf.titlebar.button[i].mouse = xcalloc(conf.titlebar.button[i].nmouse, sizeof(MouseBinding));
                    mouse_section(conf.titlebar.button[i].mouse, mouse);
               }

               free(mouse);

               /* Multi line section */
               line = fetch_section(button[i], "line");

               if((conf.titlebar.button[i].nlines = fetch_section_count(line)) > 0)
               {
                    conf.titlebar.button[i].linecoord = xcalloc(conf.titlebar.button[i].nlines, sizeof(XSegment));

                    for(j = 0; j < conf.titlebar.button[i].nlines; ++j)
                    {
                         opt = fetch_opt(line[j], "0", "coord");
                         conf.titlebar.button[i].linecoord[j].x1 = opt[0].num;
                         conf.titlebar.button[i].linecoord[j].y1 = opt[1].num;
                         conf.titlebar.button[i].linecoord[j].x2 = opt[2].num;
                         conf.titlebar.button[i].linecoord[j].y2 = opt[3].num;
                         free(opt);
                    }
               }

               free(line);
          }
     }
     free(button);

     return;
}

static void
conf_layout_section(void)
{
     int i;
     char *tmp = NULL, *p;
     struct conf_sec *layouts, **layout;

     /* Set conf.layout NULL for conf reload */
     for(i = 0; i < NUM_OF_LAYOUT; ++i)
     {
          conf.layout[i].symbol = NULL;
          conf.layout[i].func = NULL;
     }

     layouts = fetch_section_first(NULL, "layouts");

     conf.layout_button_width    = fetch_opt_first(layouts, "O", "layout_button_width").num;
     conf.border.layout          = fetch_opt_first(layouts, "false", "border").bool;
     conf.colors.layout_fg       = fetch_opt_first(layouts, "#ffffff", "fg").str;
     conf.colors.layout_bg       = getcolor((fetch_opt_first(layouts, "#000000", "bg").str));
     conf.keep_layout_geo        = fetch_opt_first(layouts, "false", "keep_layout_geo").bool;
     conf.selected_layout_symbol = fetch_opt_first(layouts, "*", "selected_layout_symbol").str;

     if((tmp = fetch_opt_first(layouts, "menu", "system").str) && !strcmp(tmp, "menu"))
          conf.layout_system = True;

     if((tmp = fetch_opt_first(layouts, "right", "placement").str) && !strcmp(tmp, "left"))
          conf.layout_placement = True;

     layout = fetch_section(layouts, "layout");
     conf.nlayout = fetch_section_count(layout);

     if(conf.nlayout > NUM_OF_LAYOUT || !(conf.nlayout))
     {
          warnx("configuration : Too many or no layouts (%d).", conf.nlayout);
          conf.nlayout          = 1;
          conf.layout[0].symbol = xstrdup("TILE");
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
               if(!name_to_func((p = fetch_opt_first(layout[i], "tile", "type").str), layout_list))
                    warnx("configuration : Unknown Layout type : \"%s\".", p);
               else
               {
                    if(conf.layout_system && conf.nlayout > 1)
                    {
                         menu_new_item(&menulayout.item[i], fetch_opt_first(layout[i], "", "symbol").str,
                                   uicb_set_layout, p);

                         menulayout.item[i].check = name_to_func("check_layout", func_list);
                    }

                    conf.layout[i].symbol = fetch_opt_first(layout[i], "TILE (default)", "symbol").str;
                    conf.layout[i].func = name_to_func(p, layout_list);
                    conf.layout[i].type = p;
               }
          }
     }
     free(layout);

     return;
}

static void
conf_tag_section(void)
{
     int i, j, k, l = 0, n, sc, bar_pos;
     char *tmp;
     char *position;
     struct conf_sec *sec, *def_tag, **tag, **mouse;

     sec = fetch_section_first(NULL, "tags");

     conf.tag_round               = fetch_opt_first(sec, "false", "tag_round").bool;
     conf.tag_auto_prev           = fetch_opt_first(sec, "true", "tag_auto_prev").bool;
     conf.colors.tagselfg         = fetch_opt_first(sec, "#ffffff", "sel_fg").str;
     conf.colors.tagselbg         = getcolor(fetch_opt_first(sec, "#000000", "sel_bg").str);
     conf.colors.tagurfg          = fetch_opt_first(sec, "#000000", "urgent_fg").str;
     conf.colors.tagurbg          = getcolor(fetch_opt_first(sec, "#DD1111", "urgent_bg").str);
     conf.colors.tag_occupied_bg  = getcolor(fetch_opt_first(sec, "#222222", "occupied_bg").str);
     conf.colors.tag_occupied_fg  = fetch_opt_first(sec, conf.colors.text, "occupied_fg").str;
     conf.border.tag              = fetch_opt_first(sec, "false", "border").bool;
     conf.tagautohide             = fetch_opt_first(sec, "false", "autohide").bool;
     conf.tagnamecount            = fetch_opt_first(sec, "false", "name_count").bool;
     conf.tag_expose_name         = fetch_opt_first(sec, "EXPOSE", "expose_name").str;
     conf.expose_layout           = fetch_opt_first(sec, "tile_grid_vertical", "expose_layout").str;

     def_tag = fetch_section_first(sec, "default_tag");

     position = fetch_opt_first(def_tag, "top", "infobar_position").str;
     if(!strcmp(position, "none")
            || !strcmp(position, "hide")
            || !strcmp(position, "hidden"))
          bar_pos = IB_Hide;
     else if(!strcmp(position, "bottom")
            || !strcmp(position, "down"))
          bar_pos = IB_Bottom;
     else
          bar_pos = IB_Top;

     /* If there is no tag in the conf or more than
      * MAXTAG (36) print an error and create only one.
      */
     Tag default_tag = { fetch_opt_first(def_tag, "new tag", "name").str, NULL, 1,
                         fetch_opt_first(def_tag, "0.6", "mwfact").fnum,
                         fetch_opt_first(def_tag, "1", "nmaster").num,
                         False, fetch_opt_first(def_tag, "False", "resizehint").bool,
                         False, False, False, bar_pos, bar_pos,
                         layout_name_to_struct(conf.layout, fetch_opt_first(def_tag, "tile", "layout").str, conf.nlayout, layout_list),
                         0, NULL, 0, False };

     conf.default_tag = default_tag;

     /* Mouse button action on tag */
     conf.mouse_tag_action[TagSel] =
          char_to_button(fetch_opt_first(sec, "1", "mouse_button_tag_sel").str, mouse_button_list);
     conf.mouse_tag_action[TagTransfert] =
          char_to_button(fetch_opt_first(sec, "2", "mouse_button_tag_transfert").str, mouse_button_list);
     conf.mouse_tag_action[TagAdd] =
          char_to_button(fetch_opt_first(sec, "3", "mouse_button_tag_add").str, mouse_button_list);
     conf.mouse_tag_action[TagNext]  =
          char_to_button(fetch_opt_first(sec, "4", "mouse_button_tag_next").str, mouse_button_list);
     conf.mouse_tag_action[TagPrev]  =
          char_to_button(fetch_opt_first(sec, "5", "mouse_button_tag_prev").str, mouse_button_list);

     sc = screen_count();

     /* Alloc all */
     conf.ntag  = xcalloc(sc, sizeof(*conf.ntag));
     tags       = xcalloc(sc, sizeof(*tags));
     seltag     = xcalloc(sc, sizeof(*seltag));
     prevseltag = xcalloc(sc, sizeof(*prevseltag));

     for(i = 0; i < sc; ++i)
          seltag[i] = 1;

     tag = fetch_section(sec, "tag");

     n = fetch_section_count(tag);

     for(i = 0; i < sc; ++i)
          tags[i] = xcalloc(n + 2, sizeof(Tag));

     for(i = 0; i < n; i++)
     {
          j = fetch_opt_first(tag[i], "-1", "screen").num;

          if(j < 0 || j > sc - 1)
               j = -1;

          for(k = ((j == -1) ? 0 : j);
              ((j == -1) ? (k < sc) : !l);
              ((j == -1) ? ++k : --l))
          {
               ++conf.ntag[k];
               tags[k][conf.ntag[k]].name       = fetch_opt_first(tag[i], fetch_opt_first(def_tag, "", "name").str, "name").str;
               tags[k][conf.ntag[k]].mwfact     = fetch_opt_first(tag[i], fetch_opt_first(def_tag, "0.65", "mwfact").str, "mwfact").fnum;
               tags[k][conf.ntag[k]].nmaster    = fetch_opt_first(tag[i], fetch_opt_first(def_tag, "1", "nmaster").str, "nmaster").num;
               tags[k][conf.ntag[k]].resizehint = fetch_opt_first(tag[i], fetch_opt_first(def_tag, "false", "resizehint").str, "resizehint").bool;
               tags[k][conf.ntag[k]].abovefc    = fetch_opt_first(tag[i], "false", "abovefc").bool;

               tmp = fetch_opt_first(tag[i], fetch_opt_first(def_tag, "top", "infobar_position").str, "infobar_position").str;

               if(!strcmp(tmp ,"none") || !strcmp(tmp, "hide") || !strcmp(tmp, "hidden"))
                    tags[k][conf.ntag[k]].barpos = IB_Hide;
               else if(!strcmp(tmp, "bottom") || !strcmp(tmp, "down"))
                    tags[k][conf.ntag[k]].barpos = IB_Bottom;
               else
                    tags[k][conf.ntag[k]].barpos = IB_Top;

               tags[k][conf.ntag[k]].layout = layout_name_to_struct(conf.layout,
                         fetch_opt_first(tag[i], fetch_opt_first(def_tag, "tile", "layout").str, "layout").str,
                         conf.nlayout,
                         layout_list);

               /* Multi mouse sections */
               mouse = fetch_section(tag[i], "mouse");

               if((tags[k][conf.ntag[k]].nmouse = fetch_section_count(mouse)))
               {
                    tags[k][conf.ntag[k]].mouse = xcalloc(tags[k][conf.ntag[k]].nmouse, sizeof(MouseBinding));
                    mouse_section(tags[k][conf.ntag[k]].mouse, mouse);
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

     return;
}

static void
conf_rule_section(void)
{
     int i;
     struct conf_sec *rules, **rule;

     if ((rules = fetch_section_first(NULL, "rules")) == NULL)
         return;

     rule = fetch_section(rules, "rule");

     CHECK((conf.nrule = fetch_section_count(rule)));

     conf.rule = xcalloc(conf.nrule, sizeof(Rule));

     for(i = 0; i < conf.nrule; ++i)
     {
          conf.rule[i].class         = fetch_opt_first(rule[i], "", "class").str;
          conf.rule[i].instance      = fetch_opt_first(rule[i], "", "instance").str;
          conf.rule[i].role          = fetch_opt_first(rule[i], "", "role").str;
          conf.rule[i].screen        = fetch_opt_first(rule[i], "-1", "screen").num;
          conf.rule[i].tag           = fetch_opt_first(rule[i], "-1", "tag").num;
          conf.rule[i].free          = fetch_opt_first(rule[i], "false", "free").bool;
          conf.rule[i].max           = fetch_opt_first(rule[i], "false", "max").bool;
          conf.rule[i].ignoretags    = fetch_opt_first(rule[i], "false", "ignoretags").bool;
          conf.rule[i].follow_client = fetch_opt_first(rule[i], "false", "follow_client").bool;
     }

     free(rule);

     return;
}

static void
conf_menu_section(void)
{
     char *tmp2;
     int i, j;
     struct conf_sec *menu, **set_menu, **item;

     menu = fetch_section_first(NULL, "menu");

     set_menu = fetch_section(menu, "set_menu");

     CHECK((conf.nmenu = fetch_section_count(set_menu)));

     conf.menu = xcalloc(conf.nmenu, sizeof(Menu));

     for(i = 0; i < conf.nmenu; ++i)
     {

          conf.menu[i].name = fetch_opt_first(set_menu[i], "menu_wname", "name").str;

          if(!(conf.menu[i].place_at_mouse = fetch_opt_first(set_menu[i], "true", "place_at_mouse").bool))
          {
               conf.menu[i].x = fetch_opt_first(set_menu[i], "0", "x").num;
               conf.menu[i].y = fetch_opt_first(set_menu[i], "0", "y").num;
          }

          tmp2 = fetch_opt_first(set_menu[i], "center", "align").str;

          if(!strcmp(tmp2 ,"left"))
               conf.menu[i].align = MA_Left;
          else if(!strcmp(tmp2, "right"))
               conf.menu[i].align = MA_Right;
          else
               conf.menu[i].align = MA_Center;

          conf.menu[i].colors.focus.bg  = getcolor(fetch_opt_first(set_menu[i], "#000000", "bg_focus").str);
          conf.menu[i].colors.focus.fg  = fetch_opt_first(set_menu[i], "#ffffff", "fg_focus").str;
          conf.menu[i].colors.normal.bg = getcolor(fetch_opt_first(set_menu[i], "#000000", "bg_normal").str);
          conf.menu[i].colors.normal.fg = fetch_opt_first(set_menu[i], "#ffffff", "fg_normal").str;

          item = fetch_section(set_menu[i], "item");

          if((conf.menu[i].nitem = fetch_section_count(item)))
          {
               conf.menu[i].item = xcalloc(conf.menu[i].nitem, sizeof(MenuItem));
               for(j = 0; j < conf.menu[i].nitem; ++j)
               {
                    conf.menu[i].item[j].name = fetch_opt_first(item[j], "item_wname", "name").str;
                    conf.menu[i].item[j].func = name_to_func(fetch_opt_first(item[j], "", "func").str, func_list);
                    conf.menu[i].item[j].cmd  = fetch_opt_first(item[j], "", "cmd").str;
                    conf.menu[i].item[j].check = name_to_func(fetch_opt_first(item[j], "", "check").str, func_list);
                    conf.menu[i].item[j].submenu = fetch_opt_first(item[j], "", "submenu").str;
               }
          }
          free(item);
     }
     free(set_menu);

     return;
}

static void
conf_launcher_section(void)
{
     int i;
     struct conf_sec *launcher, **set_launcher;

     launcher = fetch_section_first(NULL, "launcher");
     set_launcher = fetch_section(launcher, "set_launcher");

     CHECK((conf.nlauncher = fetch_section_count(set_launcher)));

     conf.launcher = xcalloc(conf.nlauncher, sizeof(Launcher));

     for(i = 0; i < conf.nlauncher; ++i)
     {
          conf.launcher[i].name    = fetch_opt_first(set_launcher[i], "launcher", "name").str;
          conf.launcher[i].prompt  = fetch_opt_first(set_launcher[i], "Exec:", "prompt").str;
          conf.launcher[i].command = fetch_opt_first(set_launcher[i], "exec", "command").str;
          conf.launcher[i].width   = fetch_opt_first(set_launcher[i], "0", "width_limit").num;
          conf.launcher[i].nhisto  = 1;
     }
     free(set_launcher);

     return;
}

static void
conf_keybind_section(void)
{
     int i;
     size_t j;
     struct conf_sec *sec, **ks;
     struct opt_type *opt;

     sec = fetch_section_first(NULL, "keys");
     ks = fetch_section(sec, "key");

     conf.nkeybind = fetch_section_count(ks);
     keys = xcalloc(conf.nkeybind, sizeof(Key));

     for(i = 0; i < conf.nkeybind; ++i)
     {
          opt = fetch_opt(ks[i], "", "mod");

          for(j = 0; j < fetch_opt_count(opt); ++j)
               keys[i].mod |= char_to_modkey(opt[j].str, key_list);

          free(opt);

          keys[i].keysym = XStringToKeysym(fetch_opt_first(ks[i], "None", "key").str);

          keys[i].func = name_to_func(fetch_opt_first(ks[i], "", "func").str, func_list);

          if(keys[i].func == NULL)
          {
               warnx("configuration : Unknown Function \"%s\".", fetch_opt_first(ks[i], "", "func").str);
               keys[i].func = uicb_spawn;
          }

          keys[i].cmd = fetch_opt_first(ks[i], "", "cmd").str;
     }

     free(ks);

     return;
}

/** Configuration initialization
*/
void
init_conf(void)
{
     if(get_conf(conf.confpath) == -1)
     {
          warnx("parsing configuration file (%s) failed.", conf.confpath);
          sprintf(conf.confpath, "%s/wmfs/wmfsrc", XDG_CONFIG_DIR);
          warnx("Use the default configuration (%s).", conf.confpath);
          if(get_conf(conf.confpath) == -1)
               errx(1, "parsing configuration file (%s) failed.", conf.confpath);

     }

     conf_misc_section();
     conf_bar_section();
     conf_root_section();
     conf_client_section();
     conf_layout_section();
     conf_tag_section();
     conf_rule_section();
     conf_menu_section();
     conf_launcher_section();
     conf_keybind_section();

     print_unused(NULL);

     return;
}


