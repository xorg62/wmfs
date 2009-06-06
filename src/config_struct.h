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
char final_path[128];

cfg_opt_t misc_opts[] =
{
     CFG_STR("font",                "sans-9",  CFGF_NONE),
     CFG_BOOL("raisefocus",         cfg_false, CFGF_NONE),
     CFG_BOOL("raiseswitch",        cfg_true,  CFGF_NONE),
     CFG_BOOL("resize_transparent", cfg_false, CFGF_NONE),
     CFG_BOOL("move_transparent",   cfg_false, CFGF_NONE),
     CFG_END()
};

cfg_opt_t mouse_button_opts[] =
{
     CFG_INT("tag",    -1,        CFGF_NONE),
     CFG_INT("screen", -1,         CFGF_NONE),
     CFG_STR("button", "Button1", CFGF_NONE),
     CFG_STR("func",   "",        CFGF_NONE),
     CFG_STR("cmd",    "",        CFGF_NONE),
     CFG_END()
};

cfg_opt_t bar_opts[] =
{
     CFG_STR("bg",        "#090909",         CFGF_NONE),
     CFG_STR("fg",        "#6289A1",         CFGF_NONE),
     CFG_INT("height",    -1,                CFGF_NONE),
     CFG_BOOL("border",   cfg_false,         CFGF_NONE),
     CFG_SEC("mouse",     mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t root_opts[] =
{
     CFG_STR("background_command", "",                CFGF_NONE),
     CFG_SEC("mouse",              mouse_button_opts, CFGF_MULTI),
     CFG_END()
};

/* CLIENT {{{ */

cfg_opt_t line_opts[] =
{
     CFG_INT_LIST("coord", "{0, 0, 0, 0}", CFGF_NONE),
     CFG_END()
};

cfg_opt_t button_opts[] =
{
     CFG_SEC("mouse", mouse_button_opts, CFGF_MULTI),
     CFG_SEC("line", line_opts, CFGF_MULTI),
     CFG_END()
};

cfg_opt_t titlebar_opts[] =
{
     CFG_INT("height",         0,                 CFGF_NONE),
     CFG_BOOL("stipple",       cfg_false,         CFGF_NONE),
     CFG_STR("stipple_normal", "-1",              CFGF_NONE),
     CFG_STR("stipple_focus",  "-1",              CFGF_NONE),
     CFG_STR("fg_normal",      "#FFFFFF",         CFGF_NONE),
     CFG_STR("fg_focus",       "#FFFFFF",         CFGF_NONE),
     CFG_SEC("mouse",          mouse_button_opts, CFGF_MULTI),
     CFG_SEC("button",         button_opts,       CFGF_MULTI),
     CFG_END()
};

cfg_opt_t client_opts[]=
{
     CFG_BOOL("place_at_mouse",       cfg_false,          CFGF_NONE),
     CFG_BOOL("border_shadow",        cfg_false,          CFGF_NONE),
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

/* }}} */

/* TAGS {{{ */

cfg_opt_t layout_opts[] =
{
     CFG_STR("type",   "", CFGF_NONE),
     CFG_STR("symbol", "", CFGF_NONE),
     CFG_END()
};

cfg_opt_t layouts_opts[] =
{
     CFG_STR("fg",      "#FFFFFF",   CFGF_NONE),
     CFG_STR("bg",      "#292929",   CFGF_NONE),
     CFG_BOOL("border", cfg_false,   CFGF_NONE),
     CFG_STR("system",  "menu",      CFGF_NONE),
     CFG_SEC("layout",  layout_opts, CFGF_MULTI),
     CFG_END()
};

/* }}} */

/* TAGS {{{ */

cfg_opt_t tag_opts[] =
{
     CFG_INT("screen",           -1,           CFGF_NONE),
     CFG_STR("name",             "",           CFGF_NONE),
     CFG_FLOAT("mwfact",         0.65,         CFGF_NONE),
     CFG_INT("nmaster",          1,            CFGF_NONE),
     CFG_STR("layout",           "tile_right", CFGF_NONE),
     CFG_STR("infobar_position", "top",        CFGF_NONE),
     CFG_BOOL("resizehint",      cfg_false,    CFGF_NONE),
     CFG_STR_LIST("clients",     "{}",     CFGF_NONE),
     CFG_END()
};

cfg_opt_t tags_opts[] =
{
     CFG_BOOL("tag_round",         cfg_false, CFGF_NONE),
     CFG_BOOL("global_resizehint", cfg_false, CFGF_NONE),
     CFG_STR("occupied_bg",        "#003366", CFGF_NONE),
     CFG_STR("sel_fg",             "#FFFFFF", CFGF_NONE),
     CFG_STR("sel_bg",             "#354B5C", CFGF_NONE),
     CFG_BOOL("border",            cfg_false, CFGF_NONE),
     CFG_SEC("tag",                tag_opts,  CFGF_MULTI),
     CFG_END()
};

/* }}} */

/* MENU {{{ */

cfg_opt_t menu_items_opts[] =
{
     CFG_STR("name", "item_wname", CFGF_NONE),
     CFG_STR("func", "",           CFGF_NONE),
     CFG_STR("cmd",  "",           CFGF_NONE),
     CFG_END()
};

cfg_opt_t menus_opts[] =
{
     CFG_STR("name",            "menu_wname",     CFGF_NONE),
     CFG_BOOL("place_at_mouse", cfg_true,         CFGF_NONE),
     CFG_INT("x",               0,                CFGF_NONE),
     CFG_INT("y",               0,                CFGF_NONE),
     CFG_STR("fg_normal",       "#ffffff",        CFGF_NONE),
     CFG_STR("bg_normal",       "#000000",        CFGF_NONE),
     CFG_STR("fg_focus",        "#ffffff",        CFGF_NONE),
     CFG_STR("bg_focus",        "#000000",        CFGF_NONE),
     CFG_SEC("item",            menu_items_opts,  CFGF_MULTI),
     CFG_END()
};

cfg_opt_t menu_opts[] =
{
     CFG_SEC("set_menu", menus_opts, CFGF_MULTI),
     CFG_END()
};

/* }}} */

/* LAUNCHER {{{ */

cfg_opt_t launchers_opts[] =
{
     CFG_STR("name",    "launcher", CFGF_NONE),
     CFG_STR("prompt",  "Execute:", CFGF_NONE),
     CFG_STR("command", "exec",     CFGF_NONE),
     CFG_END()
};

cfg_opt_t launcher_opts[] =
{
     CFG_SEC("set_launcher", launchers_opts, CFGF_MULTI),
     CFG_END()
};

/* }}} */

/* KEYBIND {{{ */

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

/* }}} */

/* ALIAS {{{ */

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

/* }}} */

cfg_opt_t opts[] =
{
     CFG_SEC("misc",     misc_opts,      CFGF_NONE),
     CFG_SEC("alias",    alias_opts,     CFGF_NONE),
     CFG_SEC("root",     root_opts,      CFGF_NONE),
     CFG_SEC("client",   client_opts,    CFGF_NONE),
     CFG_SEC("bar",      bar_opts,       CFGF_NONE),
     CFG_SEC("layouts",  layouts_opts,   CFGF_NONE),
     CFG_SEC("tags",     tags_opts,      CFGF_NONE),
     CFG_SEC("menu",     menu_opts,      CFGF_NONE),
     CFG_SEC("launcher", launcher_opts,  CFGF_NONE),
     CFG_SEC("keys",     keys_opts,      CFGF_NONE),
     CFG_END()
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

