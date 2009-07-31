/*
*      config.c
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

#include "config_struct.h"


void
conf_init_func_list(void)
{
     int i;

     func_name_list_t tmp_list[] =
          {
               {"spawn",                   uicb_spawn },
               {"client_kill",             uicb_client_kill },
               {"client_prev",             uicb_client_prev },
               {"client_next",             uicb_client_next },
               {"client_move",             uicb_client_move },
               {"client_resize",           uicb_client_resize },
               {"client_swap_next",        uicb_client_swap_next },
               {"client_swap_prev",        uicb_client_swap_prev },
               {"client_screen_next",      uicb_client_screen_next },
               {"client_screen_prev",      uicb_client_screen_prev },
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

     func_list = emalloc(LEN(tmp_list), sizeof(func_name_list_t));

     for(i = 0; i < LEN(tmp_list); ++i)
          func_list[i] = tmp_list[i];

     return;
}


/* The following function are the
   different configuration section. {{{
*/
void
mouse_section(MouseBinding mb[], cfg_t *cfg, int ns)
{
     int i;
     cfg_t *tmp;

     for(i = 0; i < ns; ++i)
     {
          tmp          = cfg_getnsec(cfg, "mouse", i);
          mb[i].tag    = cfg_getint(tmp, "tag");
          mb[i].screen = cfg_getint(tmp, "screen");
          mb[i].button = char_to_button(cfg_getstr(tmp, "button"), mouse_button_list);
          mb[i].func   = name_to_func(cfg_getstr(tmp, "func"), func_list);
          mb[i].cmd    = _strdup(alias_to_str(cfg_getstr(tmp, "cmd")));
     }

     return;
}

void
conf_alias_section(cfg_t *cfg_a)
{
     int i;

     if(cfg_size(cfg_a, "alias") < 256)
          for(i = 0; i < cfg_size(cfg_a, "alias"); ++i)
          {
               cfgtmp                = cfg_getnsec(cfg_a, "alias", i);
               conf.alias[i].name    = _strdup(cfg_title(cfgtmp));
               conf.alias[i].content = _strdup(cfg_getstr(cfgtmp, "content"));
          }
     else
     {
          fprintf(stderr,"WMFS Configuration: Too many alias (%d) !\n", cfg_size(cfg_a, "alias"));
          exit(EXIT_FAILURE);
     }

     return;
}

void
conf_misc_section(cfg_t *cfg_m)
{
     conf.font          = alias_to_str(_strdup(cfg_getstr(cfg_m, "font")));
     conf.raisefocus    = cfg_getbool(cfg_m, "raisefocus");
     conf.raiseswitch   = cfg_getbool(cfg_m, "raiseswitch");
     conf.focus_fmouse  = cfg_getbool(cfg_m, "focus_follow_mouse");

     if(cfg_getint(cfg_m, "pad") > 24 || cfg_getint(cfg_m, "pad") < 1)
     {
          fprintf(stderr, "WMFS Configuration: pad value (%ld) incorrect.\n",
                  cfg_getint(cfg_m, "pad"));

          conf.pad = 12;
     }
     else
          conf.pad = cfg_getint(cfg_m, "pad");

     return;
}

void
conf_bar_section(cfg_t *cfg_b)
{
     conf.border.bar  = cfg_getbool(cfg_b, "border");
     conf.bars.height = cfg_getint(cfg_b, "height");
     conf.colors.bar  = getcolor(alias_to_str(cfg_getstr(cfg_b, "bg")));
     conf.colors.text = _strdup(alias_to_str(cfg_getstr(cfg_b, "fg")));


     if((conf.bars.nmouse = cfg_size(cfg_b, "mouse")))
     {
          conf.bars.mouse = emalloc(conf.bars.nmouse, sizeof(MouseBinding));
          mouse_section(conf.bars.mouse, cfg_b, conf.bars.nmouse);
     }


     return;
}

void
conf_root_section(cfg_t *cfg_r)
{
     conf.root.background_command = _strdup(alias_to_str(cfg_getstr(cfg_r, "background_command")));

     if((conf.root.nmouse = cfg_size(cfg_r, "mouse")))
     {
          conf.root.mouse = emalloc(conf.root.nmouse, sizeof(MouseBinding));
          mouse_section(conf.root.mouse, cfg_r, conf.root.nmouse);
     }

     return;
}

void
conf_client_section(cfg_t *cfg_c)
{
     int i, j;
     cfg_t *cfgtmp2, *cfgtmp3;

     /* Client misc */
     conf.client.borderheight        = (cfg_getint(cfg_c, "border_height")) ? cfg_getint(cfg_c, "border_height") : 1;
     conf.client.border_shadow       = cfg_getbool(cfg_c, "border_shadow");
     conf.client.place_at_mouse      = cfg_getbool(cfg_c, "place_at_mouse");
     conf.client.bordernormal        = getcolor(alias_to_str(cfg_getstr(cfg_c, "border_normal")));
     conf.client.borderfocus         = getcolor(alias_to_str(cfg_getstr(cfg_c, "border_focus")));
     conf.client.resizecorner_normal = getcolor(alias_to_str(cfg_getstr(cfg_c, "resize_corner_normal")));
     conf.client.resizecorner_focus  = getcolor(alias_to_str(cfg_getstr(cfg_c, "resize_corner_focus")));
     conf.client.mod                |= char_to_modkey(cfg_getstr(cfg_c, "modifier"), key_list);

     if((conf.client.nmouse = cfg_size(cfg_c, "mouse")))
     {
          conf.client.mouse = emalloc(conf.client.nmouse, sizeof(MouseBinding));
          mouse_section(conf.client.mouse, cfg_c, conf.client.nmouse);
     }

     /* Titlebar part {{ */
     cfgtmp                  = cfg_getsec(cfg_c, "titlebar");
     conf.titlebar.height    = cfg_getint(cfgtmp, "height");
     conf.titlebar.fg_normal = alias_to_str(cfg_getstr(cfgtmp, "fg_normal"));
     conf.titlebar.fg_focus  = alias_to_str(cfg_getstr(cfgtmp, "fg_focus"));

     /* Stipple */
     conf.titlebar.stipple.active        = cfg_getbool(cfgtmp, "stipple");

     if(!strcmp(alias_to_str(cfg_getstr(cfgtmp, "stipple_normal")), "-1"))
          conf.titlebar.stipple.colors.normal = getcolor(conf.titlebar.fg_normal);
     else
          conf.titlebar.stipple.colors.normal = getcolor(alias_to_str(cfg_getstr(cfgtmp, "stipple_normal")));

     if(!strcmp(alias_to_str(cfg_getstr(cfgtmp, "stipple_focus")), "-1"))
          conf.titlebar.stipple.colors.focus = getcolor(conf.titlebar.fg_focus);
     else
          conf.titlebar.stipple.colors.focus = getcolor(alias_to_str(cfg_getstr(cfgtmp, "stipple_focus")));

     if((conf.titlebar.nmouse = cfg_size(cfgtmp, "mouse")))
     {
          conf.titlebar.mouse = emalloc(conf.titlebar.nmouse, sizeof(MouseBinding));
          mouse_section(conf.titlebar.mouse, cfgtmp, conf.titlebar.nmouse);
     }

     /* Multi button part */
     if((conf.titlebar.nbutton = cfg_size(cfgtmp, "button")))
     {
          conf.titlebar.button = emalloc(conf.titlebar.nbutton, sizeof(Button));
          for(i = 0; i < conf.titlebar.nbutton; ++i)
          {
               cfgtmp2 = cfg_getnsec(cfgtmp, "button", i);

               /* Multi mouse section */
               if((conf.titlebar.button[i].nmouse = cfg_size(cfgtmp2, "mouse")))
               {
                    conf.titlebar.button[i].mouse = emalloc(conf.titlebar.button[i].nmouse, sizeof(MouseBinding));
                    mouse_section(conf.titlebar.button[i].mouse, cfgtmp2, conf.titlebar.button[i].nmouse);
               }

               /* Multi line section */
               if((conf.titlebar.button[i].nlines = cfg_size(cfgtmp2, "line")))
               {
                    conf.titlebar.button[i].linecoord = emalloc(conf.titlebar.button[i].nlines, sizeof(XSegment));

                    for(j = 0; j < conf.titlebar.button[i].nlines; ++j)
                    {
                         cfgtmp3 = cfg_getnsec(cfgtmp2, "line", j);
                         conf.titlebar.button[i].linecoord[j].x1 = cfg_getnint(cfgtmp3, "coord", 0);
                         conf.titlebar.button[i].linecoord[j].y1 = cfg_getnint(cfgtmp3, "coord", 1);
                         conf.titlebar.button[i].linecoord[j].x2 = cfg_getnint(cfgtmp3, "coord", 2);
                         conf.titlebar.button[i].linecoord[j].y2 = cfg_getnint(cfgtmp3, "coord", 3);
                    }
               }
          }
     }
     /* }} */

     return;
}

void
conf_layout_section(cfg_t *cfg_l)
{
     int i;

     /* Set conf.layout NULL for conf reload */
     for(i = 0; i < NUM_OF_LAYOUT; ++i)
     {
          conf.layout[i].symbol = NULL;
          conf.layout[i].func = NULL;
     }

     conf.border.layout     = cfg_getbool(cfg_l, "border");
     conf.colors.layout_fg  = _strdup(alias_to_str(cfg_getstr(cfg_l, "fg")));
     conf.colors.layout_bg  = getcolor(alias_to_str(cfg_getstr(cfg_l, "bg")));

     if(strcmp(_strdup(alias_to_str(cfg_getstr(cfg_l, "system"))), "menu") == 0)
          conf.layout_system = True;

     if((conf.nlayout = cfg_size(cfg_l, "layout")) > NUM_OF_LAYOUT
          || !(conf.nlayout = cfg_size(cfg_l, "layout")))
     {
          fprintf(stderr, "WMFS Configuration: Too many or no layouts (%d)\n", conf.nlayout);
          conf.nlayout          = 1;
          conf.layout[0].symbol = _strdup("TILE");
          conf.layout[0].func   = tile;
     }

     if(conf.layout_system && conf.nlayout > 1)
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
               cfgtmp = cfg_getnsec(cfg_l, "layout", i);
               if(!name_to_func(_strdup(cfg_getstr(cfgtmp, "type")), layout_list))
               {
                    fprintf(stderr, "WMFS Configuration: Unknow Layout type: \"%s\"\n",
                            _strdup(cfg_getstr(cfgtmp, "type")));
                    exit(EXIT_FAILURE);
               }
               else
               {
                    if(conf.layout_system && conf.nlayout > 1)
                         menu_new_item(&menulayout.item[i],
                                       _strdup(alias_to_str(cfg_getstr(cfgtmp, "symbol"))),
                                       uicb_set_layout, _strdup(cfg_getstr(cfgtmp, "type")));

                    conf.layout[i].symbol = _strdup(alias_to_str(cfg_getstr(cfgtmp, "symbol")));
                    conf.layout[i].func = name_to_func(_strdup(cfg_getstr(cfgtmp, "type")), layout_list);
               }
          }
     }

     return;
}

void
conf_tag_section(cfg_t *cfg_t)
{
     int i, j, k, l = 0, m, n;
     char *tmp;

     /* If there is no tag in the conf or more than
      * MAXTAG (32) print an error and create only one.
      */
     Tag default_tag = { "WMFS", NULL, 0,
                         0.50, 1, False, False, IB_Top,
                         layout_name_to_struct(conf.layout, "tile_right", conf.nlayout, layout_list) };

     conf.tag_round               = cfg_getbool(cfg_t, "tag_round");
     conf.colors.tagselfg         = _strdup(alias_to_str(cfg_getstr(cfg_t, "sel_fg")));
     conf.colors.tagselbg         = getcolor(alias_to_str(cfg_getstr(cfg_t, "sel_bg")));
     conf.colors.tag_occupied_bg  = getcolor(alias_to_str(cfg_getstr(cfg_t, "occupied_bg")));
     conf.border.tag              = cfg_getbool(cfg_t, "border");

     /* Alloc all */
     conf.ntag = emalloc(screen_count(), sizeof(int));
     tags      = emalloc(screen_count(), sizeof(Tag*));
     seltag    = emalloc(screen_count(), sizeof(int));

     for(i = 0; i < screen_count(); ++i)
          seltag[i] = 1;

     for(i = 0; i < screen_count(); ++i)
          tags[i] = emalloc(cfg_size(cfg_t, "tag") + 2, sizeof(Tag));

     for(i = 0; i < cfg_size(cfg_t, "tag"); ++i)
     {
          cfgtmp = cfg_getnsec(cfg_t, "tag", i);
          j = cfg_getint(cfgtmp, "screen");
          if(j < 0 || j > screen_count() - 1)
               j = -1;

          for(k = ((j == -1) ? 0 : j);
              ((j == -1) ? (k < screen_count()) : !l);
              ((j == -1) ? ++k : --l))
          {
               ++conf.ntag[k];
               tags[k][conf.ntag[k]].name       = _strdup(cfg_getstr(cfgtmp, "name"));
               tags[k][conf.ntag[k]].mwfact     = cfg_getfloat(cfgtmp, "mwfact");
               tags[k][conf.ntag[k]].nmaster    = cfg_getint(cfgtmp, "nmaster");
               tags[k][conf.ntag[k]].resizehint = cfg_getbool(cfgtmp, "resizehint");

               tmp = _strdup(cfg_getstr(cfgtmp, "infobar_position"));

               if(!strcmp(tmp ,"none")
                  || !strcmp(tmp, "hide")
                  || !strcmp(tmp, "hidden"))
                    tags[k][conf.ntag[k]].barpos = IB_Hide;
               else if(!strcmp(tmp, "bottom")
                       || !strcmp(tmp, "down"))
                    tags[k][conf.ntag[k]].barpos = IB_Bottom;
               else
                    tags[k][conf.ntag[k]].barpos = IB_Top;

               tags[k][conf.ntag[k]].layout = layout_name_to_struct(conf.layout, cfg_getstr(cfgtmp, "layout"),
                                                                    conf.nlayout, layout_list);

               /* Clients list */
               if((n = cfg_size(cfgtmp, "clients")))
               {
                    tags[k][conf.ntag[k]].nclients = n;
                    tags[k][conf.ntag[k]].clients = emalloc(n, sizeof(char *));
                    for(m = 0; m < n; ++m)
                         tags[k][conf.ntag[k]].clients[m] = _strdup(cfg_getnstr(cfgtmp, "clients", m));
               }

          }
          l = 0;
     }

     for(i = 0; i < screen_count(); ++i)
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
conf_menu_section(cfg_t *cfg_m)
{
     cfg_t *cfgtmp2;
     int i, j;

     conf.nmenu = cfg_size(cfg_m, "set_menu");
     CHECK(conf.nmenu);
     conf.menu = emalloc(conf.nmenu, sizeof(Menu));

     for(i = 0; i < conf.nmenu; ++i)
     {
          cfgtmp = cfg_getnsec(cfg_m, "set_menu", i);

          conf.menu[i].name = _strdup(cfg_getstr(cfgtmp, "name"));

          if(!(conf.menu[i].place_at_mouse = cfg_getbool(cfgtmp, "place_at_mouse")))
          {
               conf.menu[i].x = cfg_getint(cfgtmp, "x");
               conf.menu[i].y = cfg_getint(cfgtmp, "y");
          }

          conf.menu[i].colors.focus.bg  = getcolor(alias_to_str(_strdup(cfg_getstr(cfgtmp, "bg_focus"))));
          conf.menu[i].colors.focus.fg  = _strdup(alias_to_str(cfg_getstr(cfgtmp, "fg_focus")));
          conf.menu[i].colors.normal.bg = getcolor(alias_to_str(_strdup(cfg_getstr(cfgtmp, "bg_normal"))));
          conf.menu[i].colors.normal.fg = _strdup(alias_to_str(cfg_getstr(cfgtmp, "fg_normal")));

          conf.menu[i].nitem = cfg_size(cfgtmp, "item");

          if(conf.menu[i].nitem)
          {
               conf.menu[i].item = emalloc(conf.menu[i].nitem, sizeof(MenuItem));
               for(j = 0; j < cfg_size(cfgtmp, "item"); ++j)
               {
                    cfgtmp2 = cfg_getnsec(cfgtmp, "item", j);

                    conf.menu[i].item[j].name = _strdup(cfg_getstr(cfgtmp2, "name"));
                    conf.menu[i].item[j].func = name_to_func(_strdup(cfg_getstr(cfgtmp2, "func")), func_list);
                    conf.menu[i].item[j].cmd  = (!_strdup(alias_to_str((cfg_getstr(cfgtmp2, "cmd"))))
                                                 ?  NULL : _strdup(alias_to_str(cfg_getstr(cfgtmp2, "cmd"))));
               }
          }
     }

     return;
}

void
conf_launcher_section(cfg_t *cfg_l)
{
     int i;

     conf.nlauncher = cfg_size(cfg_l, "set_launcher");
     CHECK(conf.nlauncher);
     conf.launcher = emalloc(conf.nlauncher, sizeof(Launcher));

     for(i = 0; i < conf.nlauncher; ++i)
     {
          cfgtmp = cfg_getnsec(cfg_l, "set_launcher", i);

          conf.launcher[i].name     = alias_to_str(_strdup(cfg_getstr(cfgtmp, "name")));
          conf.launcher[i].prompt   = alias_to_str(_strdup(cfg_getstr(cfgtmp, "prompt")));
          conf.launcher[i].command  = alias_to_str(_strdup(cfg_getstr(cfgtmp, "command")));
     }

     return;
}

void
conf_keybind_section(cfg_t *cfg_k)
{
     int i, j;

     conf.nkeybind = cfg_size(cfg_k, "key");
     keys = emalloc(conf.nkeybind, sizeof(Key));

     for(i = 0; i < conf.nkeybind; ++i)
     {
          cfgtmp = cfg_getnsec(cfg_k, "key", i);

          for(j = 0; j < cfg_size(cfgtmp, "mod"); ++j)
               keys[i].mod |= char_to_modkey(cfg_getnstr(cfgtmp, "mod", j), key_list);

          keys[i].keysym = XStringToKeysym(cfg_getstr(cfgtmp, "key"));
          keys[i].func = name_to_func(cfg_getstr(cfgtmp, "func"), func_list);
          if(keys[i].func == NULL)
          {
               fprintf(stderr, "WMFS Configuration error: Unknow Function \"%s\"\n",
                       cfg_getstr(cfgtmp, "func"));
               exit(EXIT_FAILURE);
          }
          keys[i].cmd = (!_strdup(alias_to_str((cfg_getstr(cfgtmp, "cmd"))))
                                 ?  NULL : _strdup(alias_to_str(cfg_getstr(cfgtmp, "cmd"))));
     }
     return;
}

/* }}} */

/** Configuration initialization
*/
void
init_conf(void)
{
     int ret;

     cfg = cfg_init(opts, CFGF_NONE);
     ret = cfg_parse(cfg, conf.confpath);

     if(ret == CFG_FILE_ERROR || ret == CFG_PARSE_ERROR)
     {
          fprintf(stderr, "WMFS: parsing configuration file (%s) failed\n", conf.confpath);
          sprintf(conf.confpath, "%s/wmfs/wmfsrc", XDG_CONFIG_DIR);
          fprintf(stderr, "Use the default configuration (%s).\n", conf.confpath);
          cfg = cfg_init(opts, CFGF_NONE);
          ret = cfg_parse(cfg, conf.confpath);
     }

     conf_init_func_list();

     conf_alias_section(cfg_getsec(cfg, "alias"));
     conf_misc_section(cfg_getsec(cfg, "misc"));
     conf_bar_section(cfg_getsec(cfg, "bar"));
     conf_root_section(cfg_getsec(cfg, "root"));
     conf_client_section(cfg_getsec(cfg, "client"));
     conf_layout_section(cfg_getsec(cfg, "layouts"));
     conf_tag_section(cfg_getsec(cfg, "tags"));
     conf_menu_section(cfg_getsec(cfg, "menu"));
     conf_launcher_section(cfg_getsec(cfg, "launcher"));
     conf_keybind_section(cfg_getsec(cfg, "keys"));

     cfg_free(cfg);

     return;
}
