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

/* local function definition */
static void conf_section(void (*)(char*), char *, char *);

func_name_list_t tmp_func_list[] =
{
     {"spawn",                   uicb_spawn },
     {"client_kill",             uicb_client_kill },
     {"client_prev",             uicb_client_prev },
     {"client_next",             uicb_client_next },
     {"client_swap_next",        uicb_client_swap_next },
     {"client_swap_prev",        uicb_client_swap_prev },
     {"client_screen_next",      uicb_client_screen_next },
     {"client_screen_prev",      uicb_client_screen_prev },
     {"client_move",             uicb_client_move },
     {"client_resize",           uicb_client_resize },
     {"toggle_max",              uicb_togglemax },
     {"layout_next",             uicb_layout_next },
     {"layout_prev",             uicb_layout_prev },
     {"tag",                     uicb_tag },
     {"tag_next",                uicb_tag_next },
     {"tag_prev",                uicb_tag_prev },
     {"tag_transfert",           uicb_tagtransfert },
     {"set_mwfact",              uicb_set_mwfact },
     {"set_nmaster",             uicb_set_nmaster },
     {"quit",                    uicb_quit },
     {"toggle_infobar_position", uicb_infobar_togglepos },
     {"toggle_resizehint",       uicb_toggle_resizehint },
     {"mouse_move",              uicb_mouse_move },
     {"mouse_resize",            uicb_mouse_resize },
     {"client_raise",            uicb_client_raise },
     {"toggle_free",             uicb_togglefree },
     {"screen_select",           uicb_screen_select },
     {"screen_next",             uicb_screen_next },
     {"screen_prev",             uicb_screen_prev },
     {"reload",                  uicb_reload },
     {"launcher",                uicb_launcher },
     {"set_layout",              uicb_set_layout },
     {"menu",                    uicb_menu }
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
mouse_section(MouseBinding mb[], char *src, int ns)
{
     int i;
     char *tmp;

     for(i = 0; i < ns; ++i)
     {
          tmp          = get_nsec(src, "mouse", i);
          mb[i].tag    = get_opt(tmp, "-1", "tag").num;
          mb[i].screen = get_opt(tmp, "-1", "screen").num;
          mb[i].button = char_to_button(get_opt(tmp, "1", "button").str, mouse_button_list);
          mb[i].func   = name_to_func(get_opt(tmp, "", "func").str, func_list);
          mb[i].cmd    = get_opt(tmp, "", "cmd").str;
     }

     return;
}

void
conf_misc_section(char *src)
{
     int pad = 12;

     conf.font          = get_opt(src, "sans-9", "font").str;
     conf.raisefocus    = get_opt(src, "false", "raisefocus").bool;
     conf.raiseswitch   = get_opt(src, "false", "raiseswitch").bool;
     conf.focus_fmouse  = get_opt(src, "true", "focus_follow_mouse").bool;
     pad = get_opt(src, "12", "pad").num;

     if(pad > 24 || pad < 1)
     {
          fprintf(stderr, "WMFS Configuration: pad value (%d) incorrect.\n", pad);

          pad = 12;
     }

     conf.pad = pad;

     return;
}

void
conf_bar_section(char *src)
{
     conf.border.bar  = get_opt(src, "false", "border").bool;
     conf.bars.height = get_opt(src, "-1", "height").num;
     conf.colors.bar  = getcolor(get_opt(src, "#000000", "bg").str);
     conf.colors.text = get_opt(src, "#ffffff", "fg").str;

     if((conf.bars.nmouse = get_size_sec(src, "mouse")))
     {
          conf.bars.mouse = emalloc(conf.bars.nmouse, sizeof(MouseBinding));
          mouse_section(conf.bars.mouse, src, conf.bars.nmouse);
     }

     return;
}

void
conf_root_section(char *src)
{
     conf.root.background_command = get_opt(src, "", "background_command").str;

     if((conf.root.nmouse = get_size_sec(src, "mouse")))
     {
          conf.root.mouse = emalloc(conf.root.nmouse, sizeof(MouseBinding));
          mouse_section(conf.root.mouse, src, conf.root.nmouse);
     }

     return;
}

void
conf_client_section(char *src)
{
     int i, j, d;
     char *tmp, *tmp2, *tmp3, *p;
     opt_type *buf;

     /* Client misc */
     conf.client.borderheight        = (get_opt(src, "1", "border_height").num) ? get_opt(src, "1", "border_height").num : 1;
     conf.client.border_shadow       = get_opt(src, "false", "border_shadow").bool;
     conf.client.place_at_mouse      = get_opt(src, "false", "place_at_mouse").bool;
     conf.client.bordernormal        = getcolor(get_opt(src, "#000000", "border_normal").str);
     conf.client.borderfocus         = getcolor(get_opt(src, "#ffffff", "border_focus").str);
     conf.client.resizecorner_normal = getcolor(get_opt(src, "#222222", "resize_corner_normal").str);
     conf.client.resizecorner_focus  = getcolor(get_opt(src, "#DDDDDD", "resize_corner_focus").str);
     conf.client.mod                 |= char_to_modkey(get_opt(src, "Alt", "modifier").str, key_list);

     if((conf.client.nmouse = get_size_sec(src, "mouse")))
     {
          conf.client.mouse = emalloc(conf.client.nmouse, sizeof(MouseBinding));
          mouse_section(conf.client.mouse, src, conf.client.nmouse);
     }

     /* Titlebar part {{ */
     tmp                     = get_sec(src, "titlebar");
     conf.titlebar.height    = get_opt(tmp, "0", "height").num;
     conf.titlebar.fg_normal = get_opt(tmp, "#ffffff", "fg_normal").str;
     conf.titlebar.fg_focus  = get_opt(tmp, "#000000", "fg_focus").str;

     /* Stipple */
     conf.titlebar.stipple.active = get_opt(tmp, "false", "stipple").bool;

     if(!strcmp((p = get_opt(tmp, "-1", "stipple_normal").str), "-1"))
          conf.titlebar.stipple.colors.normal = getcolor(conf.titlebar.fg_normal);
     else
          conf.titlebar.stipple.colors.normal = getcolor(p);

     if(!strcmp((p = get_opt(tmp, "-1", "stipple_focus").str), "-1"))
          conf.titlebar.stipple.colors.focus = getcolor(conf.titlebar.fg_focus);
     else
          conf.titlebar.stipple.colors.focus = getcolor(p);

     if((conf.titlebar.nmouse = get_size_sec(tmp, "mouse")))
     {
          conf.titlebar.mouse = emalloc(conf.titlebar.nmouse, sizeof(MouseBinding));
          mouse_section(conf.titlebar.mouse, tmp, conf.titlebar.nmouse);
     }

     /* Multi button part */
     if((conf.titlebar.nbutton = get_size_sec(tmp, "button")))
     {
          conf.titlebar.button = emalloc(conf.titlebar.nbutton, sizeof(Button));
          for(i = 0; i < conf.titlebar.nbutton; ++i)
          {
               tmp2 = get_nsec(tmp, "button", i);

               /* Multi mouse section */
               if((conf.titlebar.button[i].nmouse = get_size_sec(tmp2, "mouse")))
               {
                    conf.titlebar.button[i].mouse = emalloc(conf.titlebar.button[i].nmouse, sizeof(MouseBinding));
                    mouse_section(conf.titlebar.button[i].mouse, tmp2, conf.titlebar.button[i].nmouse);
               }

               /* Multi line section */
               if((conf.titlebar.button[i].nlines = get_size_sec(tmp2, "line")))
               {
                    conf.titlebar.button[i].linecoord = emalloc(conf.titlebar.button[i].nlines, sizeof(XSegment));

                    for(j = 0; j < conf.titlebar.button[i].nlines; ++j)
                    {
                         tmp3 = get_nsec(tmp2, "line", j);
                         buf = get_list_opt(tmp3, "{0, 0, 0, 0}", "coord", &d);

                         conf.titlebar.button[i].linecoord[j].x1 = buf[0].num;
                         conf.titlebar.button[i].linecoord[j].y1 = buf[1].num;
                         conf.titlebar.button[i].linecoord[j].x2 = buf[2].num;
                         conf.titlebar.button[i].linecoord[j].y2 = buf[3].num;
                    }
               }
          }
     }
     /* }} */

     return;
}

void
conf_layout_section(char *src)
{
     int i;
     char *tmp = NULL, *p;

     /* Set conf.layout NULL for conf reload */
     for(i = 0; i < NUM_OF_LAYOUT; ++i)
     {
          conf.layout[i].symbol = NULL;
          conf.layout[i].func = NULL;
     }

     conf.border.layout     = get_opt(src, "false", "border").bool;
     conf.colors.layout_fg  = get_opt(src, "#ffffff", "fg").str;
     conf.colors.layout_bg  = getcolor((get_opt(src, "#000000", "bg").str));

     if(strcmp(get_opt(src, "menu", "system").str, "menu") == 0)
          conf.layout_system = True;

     conf.nlayout = get_size_sec(src, "layout");

     if(conf.nlayout > NUM_OF_LAYOUT || !(conf.nlayout))
     {
          fprintf(stderr, "WMFS Configuration: Too many or no layouts (%d)\n", conf.nlayout);
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

          if(!conf.layout[0].symbol
             && !conf.layout[0].func)
          {
               for(i = 0; i < conf.nlayout; ++i)
               {
                    tmp = get_nsec(src, "layout", i);

                    if(!name_to_func((p = get_opt(tmp, "tile", "type").str), layout_list))
                         fprintf(stderr, "WMFS Configuration: Unknow Layout type: \"%s\"\n", p);
                    else
                    {
                         if(conf.layout_system && conf.nlayout > 1)
                              menu_new_item(&menulayout.item[i], get_opt(tmp, "", "symbol").str,
                                            uicb_set_layout, p);

                         conf.layout[i].symbol = get_opt(tmp, "TILE (default)", "symbol").str;
                         conf.layout[i].func = name_to_func(p, layout_list);
                    }
               }
          }
     }

     return;
}

void
conf_tag_section(char *src)
{
     int i, j, k, l = 0, m, n, sc;
     char *cfgtmp, *tmp;
     opt_type *buf;

     /* If there is no tag in the conf or more than
      * MAXTAG (32) print an error and create only one.
      */
     Tag default_tag = { "WMFS", NULL, 0,
                         0.50, 1, False, False, IB_Top,
                         layout_name_to_struct(conf.layout, "tile_right", conf.nlayout, layout_list) };

     conf.tag_round               = get_opt(src, "false", "tag_round").bool;
     conf.colors.tagselfg         = get_opt(src, "#ffffff", "sel_fg").str;
     conf.colors.tagselbg         = getcolor(get_opt(src, "#000000", "sel_bg").str);
     conf.colors.tag_occupied_bg  = getcolor(get_opt(src, "#222222", "occupied_bg").str);
     conf.border.tag              = get_opt(src, "false", "border").bool;

     sc = screen_count();

     /* Alloc all */
     conf.ntag = emalloc(sc, sizeof(int));
     tags      = emalloc(sc, sizeof(Tag*));
     seltag    = emalloc(sc, sizeof(int));

     for(i = 0; i < sc; ++i)
          seltag[i] = 1;

     for(i = 0; i < sc; ++i)
          tags[i] = emalloc(get_size_sec(src, "tag") + 2, sizeof(Tag));

     for(i = 0; i < get_size_sec(src, "tag"); ++i)
     {
          /* printf("%d -> %s\n", i, (cfgtmp = get_nsec(src, "tag", i)));*/
          cfgtmp = get_nsec(src, "tag", i);
          j = get_opt(cfgtmp, "-1", "screen").num;

          if(j < 0 || j > sc - 1)
               j = -1;

          for(k = ((j == -1) ? 0 : j);
              ((j == -1) ? (k < sc) : !l);
              ((j == -1) ? ++k : --l))
          {
               ++conf.ntag[k];
               tags[k][conf.ntag[k]].name       = get_opt(cfgtmp, "", "name").str;
               tags[k][conf.ntag[k]].mwfact     = get_opt(cfgtmp, "0.65", "mwfact").fnum;
               tags[k][conf.ntag[k]].nmaster    = get_opt(cfgtmp, "1", "nmaster").num;
               tags[k][conf.ntag[k]].resizehint = get_opt(cfgtmp, "false", "resizehint").bool;

               tmp = _strdup(get_opt(cfgtmp, "top", "infobar_position").str);

               if(!strcmp(tmp ,"none") || !strcmp(tmp, "hide") || !strcmp(tmp, "hidden"))
                    tags[k][conf.ntag[k]].barpos = IB_Hide;
               else if(!strcmp(tmp, "bottom") || !strcmp(tmp, "down"))
                    tags[k][conf.ntag[k]].barpos = IB_Bottom;
               else
                    tags[k][conf.ntag[k]].barpos = IB_Top;

               tags[k][conf.ntag[k]].layout = layout_name_to_struct(conf.layout,
                                                                    get_opt(cfgtmp, "tile_right", "layout").str,
                                                                    conf.nlayout,
                                                                    layout_list);

               /* Clients list */
               buf = get_list_opt(cfgtmp, "", "clients", &n);
               if(n)
               {
                    tags[k][conf.ntag[k]].nclients = n;
                    tags[k][conf.ntag[k]].clients = emalloc(n, sizeof(char *));
                    for(m = 0; m < n; ++m)
                         tags[k][conf.ntag[k]].clients[m] = (buf[m].str) ? buf[m].str : NULL;
               }

          }
          l = 0;
     }

     for(i = 0; i < sc; ++i)
          if(!conf.ntag[i] || conf.ntag[i] > MAXTAG)
          {
               fprintf(stderr, "WMFS Configuration: Too many or no tag"
                       " (%d) in the screen %d\n", conf.ntag[i], i);

               conf.ntag[i] = 1;
               tags[i][1] = default_tag;
          }

     return;
}

void
conf_menu_section(char *src)
{
     char *tmp, *tmp2;
     int i, j;

     CHECK((conf.nmenu = get_size_sec(src, "set_menu")));

     conf.menu = calloc(conf.nmenu, sizeof(Menu));

     for(i = 0; i < conf.nmenu; ++i)
     {
          tmp = get_nsec(src, "set_menu", i);

          conf.menu[i].name = get_opt(tmp, "menu_wname", "name").str;

          if(!(conf.menu[i].place_at_mouse = get_opt(tmp, "true", "place_at_mouse").bool))
          {
               conf.menu[i].x = get_opt(tmp, "0", "x").num;
               conf.menu[i].y = get_opt(tmp, "0", "y").num;
          }

          conf.menu[i].colors.focus.bg  = getcolor(get_opt(tmp, "#000000", "bg_focus").str);
          conf.menu[i].colors.focus.fg  = get_opt(tmp, "#ffffff", "fg_focus").str;
          conf.menu[i].colors.normal.bg = getcolor(get_opt(tmp, "#000000", "bg_normal").str);
          conf.menu[i].colors.normal.fg = get_opt(tmp, "#ffffff", "fg_normal").str;

          if((conf.menu[i].nitem = get_size_sec(tmp, "item")))
          {
               conf.menu[i].item = emalloc(conf.menu[i].nitem, sizeof(MenuItem));
               for(j = 0; j < get_size_sec(tmp, "item"); ++j)
               {
                    tmp2 = get_nsec(tmp, "item", j);

                    conf.menu[i].item[j].name = get_opt(tmp2, "item_wname", "name").str;
                    conf.menu[i].item[j].func = name_to_func(get_opt(tmp2, "", "func").str, func_list);
                    conf.menu[i].item[j].cmd  = (!get_opt(tmp2, "", "cmd").str) ? NULL : get_opt(tmp2, "", "cmd").str;
               }
          }
     }

     return;
}

void
conf_launcher_section(char *src)
{
     int i;
     char *tmp;

     CHECK((conf.nlauncher = get_size_sec(src, "set_launcher")));

     conf.launcher = emalloc(conf.nlauncher, sizeof(Launcher));

     for(i = 0; i < conf.nlauncher; ++i)
     {
          tmp = get_nsec(src, "set_launcher", i);

          conf.launcher[i].name    = get_opt(tmp, "launcher", "name").str;
          conf.launcher[i].prompt  = get_opt(tmp, "Exec:", "prompt").str;
          conf.launcher[i].command = get_opt(tmp, "exec", "command").str;
     }

     return;
}

void
conf_keybind_section(char *src)
{
     int i, j, n = 0;
     char *tmp;
     opt_type *buf;

     conf.nkeybind = get_size_sec(src, "key");
     keys = emalloc(conf.nkeybind, sizeof(Key));

     for(i = 0; i < conf.nkeybind; ++i)
     {
          tmp = get_nsec(src, "key", i);
          buf = get_list_opt(tmp, "{Control}", "mod", &n);

          for(j = 0; j < n; ++j)
               keys[i].mod |= char_to_modkey(buf[j].str, key_list);

          keys[i].keysym = XStringToKeysym(get_opt(tmp, "None", "key").str);
          keys[i].func = name_to_func(get_opt(tmp, "", "func").str, func_list);

          if(keys[i].func == NULL)
          {
               fprintf(stderr, "WMFS Configuration warning: Unknow Function \"%s\"\n", get_opt(tmp, "", "func").str);
               keys[i].func = uicb_spawn;
          }

          keys[i].cmd = (!get_opt(tmp, "", "cmd").str) ? NULL : get_opt(tmp, "", "cmd").str;
     }

     return;
}

/** Configuration initialization
*/
void
init_conf(void)
{
     char *file;

     if(!(file = file_to_str(conf.confpath)))
     {
          fprintf(stderr, "WMFS: parsing configuration file (%s) failed\n", conf.confpath);
          sprintf(conf.confpath, "%s/wmfs/wmfsrc", XDG_CONFIG_DIR);
          fprintf(stderr, "Use the default configuration (%s).\n", conf.confpath);
          file = file_to_str(conf.confpath);
     }

     /* Set func_list */
     func_list = emalloc(LEN(tmp_func_list), sizeof(func_name_list_t));
     memcpy(func_list, tmp_func_list, LEN(tmp_func_list) * sizeof(func_name_list_t));

     conf_section(conf_misc_section, file, "misc");
     conf_section(conf_bar_section, file, "bar");
     conf_section(conf_root_section, file, "root");
     conf_section(conf_client_section, file, "client");
     conf_section(conf_layout_section, file, "layouts");
     conf_section(conf_tag_section, file, "tags");
     conf_section(conf_menu_section, file, "menu");
     conf_section(conf_launcher_section, file, "launcher");
     conf_section(conf_keybind_section, file, "keys");

     free(file);

     return;
}


/** Simple wrapper for calling functions and free pointer
 */
static void
conf_section(void (*func)(char*), char *src, char *name)
{
     char *sec;

     sec = get_sec(src, name);
     func(sec);
     free(sec);

     return;
}


