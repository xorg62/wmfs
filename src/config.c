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

#include "wmfs.h"

#define FILE_NAME   ".config/wmfs/wmfsrc"

cfg_t *cfg, *cfgtmp;

static cfg_opt_t misc_opts[] =
{
     CFG_STR("font",             "sans-9",  CFGF_NONE),
     CFG_BOOL("raisefocus",      cfg_false, CFGF_NONE),
     CFG_BOOL("raiseswitch",     cfg_true,  CFGF_NONE),
     CFG_END()
};

static cfg_opt_t bar_opts[] =
{
     CFG_STR("bg",        "#090909", CFGF_NONE),
     CFG_STR("fg",        "#6289A1", CFGF_NONE),
     CFG_STR("position",  "top",     CFGF_NONE),
     CFG_END()
};

static cfg_opt_t mouse_button_opts[] =
{
     CFG_INT("tag",    -1,        CFGF_NONE),
     CFG_INT("screen", 0,         CFGF_NONE),
     CFG_STR("button", "Button1", CFGF_NONE),
     CFG_STR("func",   "",        CFGF_NONE),
     CFG_STR("cmd",    "",        CFGF_NONE),
     CFG_END()
};

static cfg_opt_t root_opts[] =
{
     CFG_STR("background_command", "",                CFGF_NONE),
     CFG_SEC("mouse",              mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t titlebar_opts[] =
{
     CFG_INT("height", 0,                 CFGF_NONE),
     CFG_STR("fg",     "#FFFFFF",         CFGF_NONE),
     CFG_SEC("mouse",  mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t client_opts[]=
{
     CFG_BOOL("place_at_mouse",       cfg_false,          CFGF_NONE),
     CFG_INT("border_height",         1,                  CFGF_NONE),
     CFG_STR("border_normal",         "#354B5C",          CFGF_NONE),
     CFG_STR("border_focus",          "#6286A1",          CFGF_NONE),
     CFG_STR("resize_corner_normal",  "#ff0000",          CFGF_NONE),
     CFG_STR("resize_corner_focus",   "#ff0000",          CFGF_NONE),
     CFG_STR("modifier",              "Alt",              CFGF_NONE),
     CFG_SEC("mouse",                 mouse_button_opts,  CFGF_MULTI),
     CFG_SEC("titlebar",              titlebar_opts,      CFGF_NONE),
     CFG_END()
};

static cfg_opt_t layout_opts[] =
{
     CFG_STR("type",   "", CFGF_NONE),
     CFG_STR("symbol", "", CFGF_NONE),
     CFG_END()
};

static cfg_opt_t layouts_opts[] =
{
     CFG_STR("fg",           "#FFFFFF",   CFGF_NONE),
     CFG_STR("bg",           "#292929",   CFGF_NONE),
     CFG_SEC("layout",       layout_opts, CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t tag_opts[] =
{
     CFG_INT("screen",      -1,           CFGF_NONE),
     CFG_STR("name",        "",           CFGF_NONE),
     CFG_FLOAT("mwfact",    0.65,         CFGF_NONE),
     CFG_INT("nmaster",     1,            CFGF_NONE),
     CFG_STR("layout",      "tile_right", CFGF_NONE),
     CFG_BOOL("resizehint", cfg_false,    CFGF_NONE),
     CFG_END()
};

static cfg_opt_t tags_opts[] =
{
     CFG_BOOL("tag_round",   cfg_false, CFGF_NONE),
     CFG_STR("occupied_bg",  "#003366", CFGF_NONE),
     CFG_STR("sel_fg",       "#FFFFFF", CFGF_NONE),
     CFG_STR("sel_bg",       "#354B5C", CFGF_NONE),
     CFG_STR("border",       "#090909", CFGF_NONE),
     CFG_SEC("tag",          tag_opts,  CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t key_opts[] =
{
     CFG_STR_LIST("mod", "{Control}", CFGF_NONE),
     CFG_STR("key",      "None",      CFGF_NONE),
     CFG_STR("func",     "",          CFGF_NONE),
     CFG_STR("cmd",      "",          CFGF_NONE),
     CFG_END()
};

static cfg_opt_t keys_opts[] =
{
     CFG_SEC("key", key_opts, CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t _alias_opts[] =
{
     CFG_STR("content", "", CFGF_NONE),
     CFG_END()
};

static cfg_opt_t alias_opts[] =
{
     CFG_SEC("alias", _alias_opts, CFGF_TITLE | CFGF_MULTI),
     CFG_END()
};

static cfg_opt_t opts[] =
{
     CFG_SEC("misc",    misc_opts,      CFGF_NONE),
     CFG_SEC("alias",   alias_opts,     CFGF_NONE),
     CFG_SEC("root",    root_opts,      CFGF_NONE),
     CFG_SEC("client",  client_opts,    CFGF_NONE),
     CFG_SEC("bar",     bar_opts,       CFGF_NONE),
     CFG_SEC("layouts", layouts_opts,   CFGF_NONE),
     CFG_SEC("tags",    tags_opts,      CFGF_NONE),
     CFG_SEC("keys",    keys_opts,      CFGF_NONE),
     CFG_END()
};


static func_name_list_t func_list[] =
{
     {"spawn",                   uicb_spawn },
     {"client_kill",             uicb_client_kill },
     {"client_prev",             uicb_client_prev },
     {"client_next",             uicb_client_next },
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
     {"mouse_move",              uicb_mouse_move },
     {"mouse_resize",            uicb_mouse_resize },
     {"client_raise",            uicb_client_raise },
     {"tile_switch",             uicb_tile_switch },
     {"toggle_free",             uicb_togglefree },
     {"reload",                  uicb_reload }
};

static func_name_list_t layout_list[] =
{
     {"tile_right", tile },
     {"tile_left", tile_left },
     {"tile_top", tile_top },
     {"tile_bottom", tile_bottom },
     {"tile_grid", grid},
     {"max",  maxlayout },
     {"free", freelayout }
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
};


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
          mb[i].cmd    = strdup(alias_to_str(cfg_getstr(tmp, "cmd")));
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
               conf.alias[i].name    = strdup(cfg_title(cfgtmp));
               conf.alias[i].content = strdup(cfg_getstr(cfgtmp, "content"));
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
     conf.font          = alias_to_str(strdup(cfg_getstr(cfg_m, "font")));
     conf.raisefocus    = cfg_getbool(cfg_m, "raisefocus");
     conf.raiseswitch   = cfg_getbool(cfg_m, "raiseswitch");

     return;
}

void
conf_bar_section(cfg_t *cfg_b)
{
     conf.colors.bar  = getcolor(alias_to_str(cfg_getstr(cfg_b, "bg")));
     conf.colors.text = strdup(alias_to_str(cfg_getstr(cfg_b, "fg")));
     conf.bartop      = (strcmp(strdup(cfg_getstr(cfg_b, "position")), "top") == 0) ? True : False;

     return;
}

void
conf_root_section(cfg_t *cfg_r)
{
     conf.root.background_command = strdup(alias_to_str(cfg_getstr(cfg_r, "background_command")));
     conf.root.nmouse = cfg_size(cfg_r, "mouse");
     conf.root.mouse = emalloc(conf.root.nmouse, sizeof(MouseBinding));
     mouse_section(conf.root.mouse, cfg_r, conf.root.nmouse);

     return;
}

void
conf_client_section(cfg_t *cfg_c)
{
     /* Client  misc */
     conf.client.borderheight        = (cfg_getint(cfg_c, "border_height")) ? cfg_getint(cfg_c, "border_height") : 1;
     conf.client.place_at_mouse      = cfg_getbool(cfg_c, "place_at_mouse");
     conf.client.bordernormal        = getcolor(alias_to_str(cfg_getstr(cfg_c, "border_normal")));
     conf.client.borderfocus         = getcolor(alias_to_str(cfg_getstr(cfg_c, "border_focus")));
     conf.client.resizecorner_normal = getcolor(alias_to_str(cfg_getstr(cfg_c, "resize_corner_normal")));
     conf.client.resizecorner_focus  = getcolor(alias_to_str(cfg_getstr(cfg_c, "resize_corner_focus")));
     conf.client.mod                |= char_to_modkey(cfg_getstr(cfg_c, "modifier"), key_list);
     conf.client.nmouse              = cfg_size(cfg_c, "mouse");
     conf.client.mouse               = emalloc(conf.client.nmouse, sizeof(MouseBinding));
     mouse_section(conf.client.mouse, cfg_c, conf.client.nmouse);

     /* Titlebar part */
     cfgtmp                = cfg_getsec(cfg_c, "titlebar");
     conf.titlebar.height  = cfg_getint(cfgtmp, "height");
     conf.titlebar.fg      = alias_to_str(cfg_getstr(cfgtmp, "fg"));
     conf.titlebar.nmouse  = cfg_size(cfgtmp, "mouse");
     conf.titlebar.mouse   = emalloc(conf.titlebar.nmouse, sizeof(MouseBinding));
     mouse_section(conf.titlebar.mouse, cfgtmp, conf.titlebar.nmouse);

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

     conf.colors.layout_fg  = strdup(alias_to_str(cfg_getstr(cfg_l, "fg")));
     conf.colors.layout_bg  = getcolor(alias_to_str(cfg_getstr(cfg_l, "bg")));

     if((conf.nlayout = cfg_size(cfg_l, "layout")) > NUM_OF_LAYOUT
          || !(conf.nlayout = cfg_size(cfg_l, "layout")))
     {
          fprintf(stderr, "WMFS Configuration: Too many or no layouts (%d)\n", conf.nlayout);
          conf.nlayout          = 1;
          conf.layout[0].symbol = strdup("TILE");
          conf.layout[0].func   = tile;
     }

     if(!conf.layout[0].symbol
        && !conf.layout[0].func)
     {
          for(i = 0; i < conf.nlayout; ++i)
          {
               cfgtmp = cfg_getnsec(cfg_l, "layout", i);
               if(!name_to_func(strdup(cfg_getstr(cfgtmp, "type")), layout_list))
               {
                    fprintf(stderr, "WMFS Configuration: Unknow Layout type: \"%s\"\n",
                            strdup(cfg_getstr(cfgtmp, "type")));
                    exit(EXIT_FAILURE);
               }
               else
               {
                    conf.layout[i].symbol = strdup(alias_to_str(cfg_getstr(cfgtmp, "symbol")));
                    conf.layout[i].func = name_to_func(strdup(cfg_getstr(cfgtmp, "type")), layout_list);
               }
          }
     }

     return;
}

void
conf_tag_section(cfg_t *cfg_t)
{
     int i, j, k;

     /* If there is no tag in the conf or more than
      * MAXTAG (32) print an error and create only one.
      */
     conf.tag_round               = cfg_getbool(cfg_t, "tag_round");
     conf.colors.tagselfg         = strdup(alias_to_str(cfg_getstr(cfg_t, "sel_fg")));
     conf.colors.tagselbg         = getcolor(alias_to_str(cfg_getstr(cfg_t, "sel_bg")));
     conf.colors.tag_occupied_bg  = getcolor(alias_to_str(cfg_getstr(cfg_t, "occupied_bg")));
     conf.colors.tagbord          = getcolor(alias_to_str(cfg_getstr(cfg_t, "border")));

     /* Alloc all */
     conf.ntag = emalloc(screen_count(), sizeof(int));
     tags = emalloc(screen_count(), sizeof(Tag*));
     for(i = 0; i < screen_count(); ++i)
          tags[i] = emalloc(cfg_size(cfg_t, "tag") + 1, sizeof(Tag));

     for(i = 0; i < cfg_size(cfg_t, "tag"); ++i)
     {
          cfgtmp = cfg_getnsec(cfg_t, "tag", i);
          j = cfg_getint(cfgtmp, "screen");
          if(j < 0 || j > screen_count() - 1)
               j = -1;

          if(j == -1)
          {
               for(k = 0; k < screen_count(); ++k)
               {
                    ++conf.ntag[k];

                    tags[k][conf.ntag[k]].name       = strdup(cfg_getstr(cfgtmp, "name"));
                    tags[k][conf.ntag[k]].mwfact     = cfg_getfloat(cfgtmp, "mwfact");
                    tags[k][conf.ntag[k]].nmaster    = cfg_getint(cfgtmp, "nmaster");
                    tags[k][conf.ntag[k]].resizehint = cfg_getbool(cfgtmp, "resizehint");
                    tags[k][conf.ntag[k]].layout     = layout_name_to_struct(conf.layout,
                                                                             cfg_getstr(cfgtmp, "layout"),
                                                                             conf.nlayout,
                                                                             layout_list);
               }
          }
          else
          {
               ++conf.ntag[j];

               tags[j][conf.ntag[j]].name       = strdup(cfg_getstr(cfgtmp, "name"));
               tags[j][conf.ntag[j]].mwfact     = cfg_getfloat(cfgtmp, "mwfact");
               tags[j][conf.ntag[j]].nmaster    = cfg_getint(cfgtmp, "nmaster");
               tags[j][conf.ntag[j]].resizehint = cfg_getbool(cfgtmp, "resizehint");
               tags[j][conf.ntag[j]].layout     = layout_name_to_struct(conf.layout,
                                                                        cfg_getstr(cfgtmp, "layout"),
                                                                        conf.nlayout,
                                                                        layout_list);
          }
     }


     for(i = 0; i < screen_count(); ++i)
          if(!conf.ntag[i] || conf.ntag[i] > MAXTAG)
          {
               fprintf(stderr, "WMFS Configuration: Too many or no tag"
                       " (%d) in the screen %d\n", conf.ntag[i], i);

               conf.ntag[i] = 1;
               tags[i][1].name       = strdup("WMFS");
               tags[i][1].mwfact     = 0.50;
               tags[i][1].nmaster    = 1;
               tags[i][1].resizehint = False;
               tags[i][1].layout     = layout_name_to_struct(conf.layout,
                                                             "tile_right",
                                                             conf.nlayout,
                                                             layout_list);
          }

     seltag = emalloc(screen_count(), sizeof(int));
     for(j = 0; j < screen_count(); ++j)
          seltag[j] = 1;

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
               keys[j].mod |= char_to_modkey(cfg_getnstr(cfgtmp, "mod", j), key_list);

          keys[i].keysym = XStringToKeysym(cfg_getstr(cfgtmp, "key"));
          keys[i].func = name_to_func(cfg_getstr(cfgtmp, "func"), func_list);
          if(keys[i].func == NULL)
          {
               fprintf(stderr, "WMFS Configuration error: Unknow Function \"%s\"\n",
                       cfg_getstr(cfgtmp, "func"));
               exit(EXIT_FAILURE);
          }
          keys[i].cmd = (!strdup(alias_to_str((cfg_getstr(cfgtmp, "cmd"))))
                                 ?  NULL : strdup(alias_to_str(cfg_getstr(cfgtmp, "cmd"))));
     }

     return;
}

/* }}} */

/** Configuration initialization
*/
void
init_conf(void)
{
     char final_path[128];
     int ret;

     sprintf(final_path, "%s/%s",
             strdup(getenv("HOME")),
             strdup(FILE_NAME));

     cfg = cfg_init(opts, CFGF_NONE);
     ret = cfg_parse(cfg, final_path);

     if(ret == CFG_FILE_ERROR || ret == CFG_PARSE_ERROR)
     {
          fprintf(stderr, "WMFS: parsing configuration file (%s) failed\n", final_path);
          sprintf(final_path, "%s/wmfs/wmfsrc", XDG_CONFIG_DIR);
          fprintf(stderr, "Use the default configuration (%s).\n", final_path);
          cfg = cfg_init(opts, CFGF_NONE);
          ret = cfg_parse(cfg, final_path);
     }

     conf_alias_section(cfg_getsec(cfg, "alias"));
     conf_misc_section(cfg_getsec(cfg, "misc"));
     conf_bar_section(cfg_getsec(cfg, "bar"));
     conf_root_section(cfg_getsec(cfg, "root"));
     conf_client_section(cfg_getsec(cfg, "client"));
     conf_layout_section(cfg_getsec(cfg, "layouts"));
     conf_tag_section(cfg_getsec(cfg, "tags"));
     conf_keybind_section(cfg_getsec(cfg, "keys"));

     cfg_free(cfg);

     return;
}
