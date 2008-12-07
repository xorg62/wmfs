/*
*      config_struct.h
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
*      OF THIS SOFTWARE,# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

#define FILE_NAME   ".config/wmfs/wmfsrc"

cfg_t *cfg, *cfgtmp;

cfg_opt_t misc_opts[] =
{
     CFG_STR("font",             "sans-9",  CFGF_NONE),
     CFG_BOOL("raisefocus",      cfg_false, CFGF_NONE),
     CFG_BOOL("raiseswitch",     cfg_true,  CFGF_NONE),
     CFG_END()
};

cfg_opt_t bar_opts[] =
{
     CFG_STR("bg",        "#090909", CFGF_NONE),
     CFG_STR("fg",        "#6289A1", CFGF_NONE),
     CFG_STR("position",  "top",     CFGF_NONE),
     CFG_END()
};

cfg_opt_t mouse_button_opts[] =
{
     CFG_INT("tag",    -1,        CFGF_NONE),
     CFG_INT("screen", 0,         CFGF_NONE),
     CFG_STR("button", "Button1", CFGF_NONE),
     CFG_STR("func",   "",        CFGF_NONE),
     CFG_STR("cmd",    "",        CFGF_NONE),
     CFG_END()
};

cfg_opt_t root_opts[] =
{
     CFG_STR("background_command", "",                CFGF_NONE),
     CFG_SEC("mouse",              mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t titlebar_opts[] =
{
     CFG_INT("height", 0,                 CFGF_NONE),
     CFG_STR("fg",     "#FFFFFF",         CFGF_NONE),
     CFG_SEC("mouse",  mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t client_opts[]=
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

cfg_opt_t layout_opts[] =
{
     CFG_STR("type",   "", CFGF_NONE),
     CFG_STR("symbol", "", CFGF_NONE),
     CFG_END()
};

cfg_opt_t layouts_opts[] =
{
     CFG_STR("fg",           "#FFFFFF",   CFGF_NONE),
     CFG_STR("bg",           "#292929",   CFGF_NONE),
     CFG_SEC("layout",       layout_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t tag_opts[] =
{
     CFG_INT("screen",      -1,           CFGF_NONE),
     CFG_STR("name",        "",           CFGF_NONE),
     CFG_FLOAT("mwfact",    0.65,         CFGF_NONE),
     CFG_INT("nmaster",     1,            CFGF_NONE),
     CFG_STR("layout",      "tile_right", CFGF_NONE),
     CFG_BOOL("resizehint", cfg_false,    CFGF_NONE),
     CFG_END()
};

cfg_opt_t tags_opts[] =
{
     CFG_BOOL("tag_round",   cfg_false, CFGF_NONE),
     CFG_STR("occupied_bg",  "#003366", CFGF_NONE),
     CFG_STR("sel_fg",       "#FFFFFF", CFGF_NONE),
     CFG_STR("sel_bg",       "#354B5C", CFGF_NONE),
     CFG_STR("border",       "#090909", CFGF_NONE),
     CFG_SEC("tag",          tag_opts,  CFGF_MULTI),
     CFG_END()
};

cfg_opt_t key_opts[] =
{
     CFG_STR_LIST("mod", "{Control}", CFGF_NONE),
     CFG_STR("key",      "None",      CFGF_NONE),
     CFG_STR("func",     "",          CFGF_NONE),
     CFG_STR("cmd",      "",          CFGF_NONE),
     CFG_END()
};

cfg_opt_t keys_opts[] =
{
     CFG_SEC("key", key_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t _alias_opts[] =
{
     CFG_STR("content", "", CFGF_NONE),
     CFG_END()
};

cfg_opt_t alias_opts[] =
{
     CFG_SEC("alias", _alias_opts, CFGF_TITLE | CFGF_MULTI),
     CFG_END()
};

cfg_opt_t opts[] =
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

func_name_list_t func_list[] =
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

func_name_list_t layout_list[] =
{
     {"tile_right", tile },
     {"tile_left", tile_left },
     {"tile_top", tile_top },
     {"tile_bottom", tile_bottom },
     {"tile_grid", grid},
     {"max",  maxlayout },
     {"free", freelayout }
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

