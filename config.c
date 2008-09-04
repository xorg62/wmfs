#include <stdio.h>
#include <confuse.h>
#include <string.h>

#include "config.h"

#define FILE_NAME   ".wmfsrc"

func_name_list_t func_list[] = {
     {"spawn", spawn},
     {"killclient", killclient},
     {"wswitch", wswitch},
     {"tagswitch", tagswitch},
     {"togglemax", togglemax},
     {"keymovex", keymovex},
     {"keymovey", keymovey},
     {"keyresize", keyresize},
     {"layoutswitch",layoutswitch},
     {"tag", tag},
     {"tagtransfert", tagtransfert}
};

key_name_list_t key_list[] = {
     {"Control", ControlMask},
     {"Shift", ShiftMask},
     {"Lock", LockMask},
     {"Control", ControlMask},
     {"Alt", Mod1Mask},
     {"Mod2", Mod2Mask},
     {"Mod3", Mod3Mask},
     {"Mod4", Mod4Mask},
     {"Mod5", Mod5Mask},
     {NULL, NoSymbol}
};

mouse_button_list_t mouse_button_list[] = {
     {"Button1", Button1},
     {"Button2", Button2},
     {"Button3", Button3},
     {"Button4", Button4},
     {"Button5", Button5}
};

void*
name_to_func(char *name) {
     int i;
     if(name)
          for(i=0; func_list[i].name ; ++i)
               if(!strcmp(name, func_list[i].name))
                    return func_list[i].func;
     return NULL;
}


unsigned long
char_to_modkey(char *name) {
     int i;
     if(name)
          for(i=0; key_list[i].name; ++i)
               if(!strcmp(name, key_list[i].name))
                    return key_list[i].keysym;
     return NoSymbol;
}

unsigned int
char_to_button(char *name) {
     int i;
     if(name)
          for(i=0; mouse_button_list[i].name; ++i)
               if(!strcmp(name, mouse_button_list[i].name))
                    return mouse_button_list[i].button;
     return 0;
}

void
init_conf(void) {

     static cfg_opt_t misc_opts[] = {

          CFG_STR("font",              "*-fixed-medium-*-12-*",  CFGF_NONE),
          CFG_BOOL("raisefocus",        cfg_false,               CFGF_NONE),
          CFG_BOOL("raiseswitch",       cfg_true,                CFGF_NONE),
          CFG_INT("border_height",      1,                       CFGF_NONE),
          CFG_INT("titlebar_height",    12,                      CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t colors_opts[] = {

          CFG_INT("border_normal", 0x354B5C,  CFGF_NONE),
          CFG_INT("border_focus",  0x6286A1,  CFGF_NONE),
          CFG_INT("bar",           0x090909,  CFGF_NONE),
          CFG_INT("button",        0x354B5C,  CFGF_NONE),
          CFG_INT("text",          0x6289A1,  CFGF_NONE),
          CFG_INT("tag_sel_fg",    0xFFFFFF,  CFGF_NONE),
          CFG_INT("tag_sel_bg",    0x354B5C,  CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t layout_opts[] = {

          CFG_STR_LIST("free","[Free]", CFGF_NONE),
          CFG_STR_LIST("tile","[Tile]", CFGF_NONE),
          CFG_STR_LIST("max","[Max]", CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t tag_opts[] = {

          CFG_STR_LIST("tag", "{Tag}", CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t key_opts[] = {

          CFG_STR_LIST("mod","{Control}", CFGF_NONE),
          CFG_STR("key", "None", CFGF_NONE),
          CFG_STR("func", "", CFGF_NONE),
          CFG_STR("cmd",  "", CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t keys_opts[] = {

          CFG_SEC("key", key_opts, CFGF_MULTI),
          CFG_END()
     };

     static cfg_opt_t mouse_button_opts[] = {

          CFG_STR("button", "Button1", CFGF_NONE),
          CFG_STR("func",   "",        CFGF_NONE),
          CFG_STR("cmd",    "",        CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t button_opts[] = {

          CFG_STR("text", "", CFGF_NONE),
          CFG_SEC("mouse",    mouse_button_opts, CFGF_MULTI),
          CFG_INT("fg_color", 0x000000, CFGF_NONE),
          CFG_INT("bg_color", 0xFFFFFF, CFGF_NONE),
          CFG_INT("x", 0, CFGF_NONE),
          CFG_END()
     };

     static cfg_opt_t buttons_opts[] = {

          CFG_SEC("button", button_opts, CFGF_MULTI),
          CFG_END()
     };

     static cfg_opt_t opts[] = {

          CFG_SEC("misc",    misc_opts,    CFGF_NONE),
          CFG_SEC("colors",  colors_opts,  CFGF_NONE),
          CFG_SEC("layout",  layout_opts,  CFGF_NONE),
          CFG_SEC("tag",     tag_opts,     CFGF_NONE),
          CFG_SEC("keys",    keys_opts,    CFGF_NONE),
          CFG_SEC("buttons", buttons_opts, CFGF_NONE),
          CFG_END()
     };

     cfg_t *cfg;
     cfg_t *cfg_misc;
     cfg_t *cfg_colors;
     cfg_t *cfg_layout;
     cfg_t *cfg_tag;
     cfg_t *cfg_keys;
     cfg_t *cfg_buttons;
     cfg_t *cfgtmp, *cfgtmp2, *cfgtmp3;
     char final_path[100];
     int ret, i, j, l;

     sprintf(final_path,"%s/%s",strdup(getenv("HOME")),strdup(FILE_NAME));

     cfg = cfg_init(opts, CFGF_NONE);

     ret = cfg_parse(cfg, final_path);

     if(ret == CFG_FILE_ERROR) {
          printf("WMFS: parsing configuration file failed\n");
          exit(1);
     }
     else if(ret == CFG_PARSE_ERROR) {
          cfg_error(cfg, "WMFS: parsing configuration file %s failed.\n", final_path);
          exit(1);
     }

     cfg_misc    = cfg_getsec(cfg, "misc");
     cfg_colors  = cfg_getsec(cfg, "colors");
     cfg_layout  = cfg_getsec(cfg, "layout");
     cfg_tag     = cfg_getsec(cfg, "tag");
     cfg_keys    = cfg_getsec(cfg, "keys");
     cfg_buttons = cfg_getsec(cfg, "buttons");

     /* misc */
     conf.font           = strdup(cfg_getstr(cfg_misc, "font"));
     conf.raisefocus     = cfg_getbool(cfg_misc,       "raisefocus");
     conf.raiseswitch    = cfg_getbool(cfg_misc,       "raiseswitch");
     conf.borderheight   = cfg_getint(cfg_misc,        "border_height");
     conf.ttbarheight    = cfg_getint(cfg_misc,        "titlebar_height");

     /* colors */
     conf.colors.bordernormal = cfg_getint(cfg_colors, "border_normal");
     conf.colors.borderfocus  = cfg_getint(cfg_colors, "border_focus");
     conf.colors.bar          = cfg_getint(cfg_colors, "bar");
     conf.colors.button       = cfg_getint(cfg_colors, "button");
     conf.colors.text         = cfg_getint(cfg_colors, "text");
     conf.colors.tagselfg     = cfg_getint(cfg_colors, "tag_sel_fg");
     conf.colors.tagselbg     = cfg_getint(cfg_colors, "tag_sel_bg");

     /* layout */
     conf.layouts.free = strdup(cfg_getstr(cfg_layout, "free"));
     conf.layouts.tile = strdup(cfg_getstr(cfg_layout, "tile"));
     conf.layouts.max  = strdup(cfg_getstr(cfg_layout, "max"));

     /* tag */
     conf.ntag = cfg_size(cfg_tag, "tag");
     for(i=0; i < cfg_size(cfg_tag, "tag"); ++i)
          conf.taglist[i] = strdup(cfg_getnstr(cfg_tag, "tag",i));

     /* keybind ('tention ça rigole plus) */
     conf.nkeybind = cfg_size(cfg_keys, "key");
     for(j = 0; j <  cfg_size(cfg_keys, "key"); ++j) {
          cfgtmp = cfg_getnsec(cfg_keys, "key", j);

          for(l = 0; l < cfg_size(cfgtmp, "mod"); ++l)
               keys[j].mod |= char_to_modkey(cfg_getnstr(cfgtmp, "mod", l));

          keys[j].keysym = XStringToKeysym(cfg_getstr(cfgtmp, "key"));
          keys[j].func = name_to_func(cfg_getstr(cfgtmp, "func"));
          if(!keys[j].func) {
               printf("WMFS Configuration: Unknow Function %s",cfg_getstr(cfgtmp, "func"));
               return;
          }
          if(!strdup(strdup(cfg_getstr(cfgtmp, "cmd"))))
               keys[j].cmd = NULL;
          else
               keys[j].cmd = strdup(strdup(cfg_getstr(cfgtmp, "cmd")));
     }

     /* button */
     conf.nbutton = cfg_size(cfg_buttons, "button");
     for(i = 0; i < conf.nbutton; ++i) {
          cfgtmp2 = cfg_getnsec(cfg_buttons, "button", i);
          for(j = 0; j < cfg_size(cfgtmp2, "mouse");  ++j) {
               cfgtmp3 = cfg_getnsec(cfgtmp2, "mouse", j);
               conf.barbutton[i].func[j] = name_to_func(cfg_getstr(cfgtmp3, "func"));
               conf.barbutton[i].cmd[j] = strdup(cfg_getstr(cfgtmp3, "cmd"));
               conf.barbutton[i].mouse[j] = char_to_button(cfg_getstr(cfgtmp3, "button"));
          }
          conf.barbutton[i].nmousesec = cfg_size(cfgtmp2, "mouse");
          conf.barbutton[i].text = strdup(cfg_getstr(cfgtmp2, "text"));
          conf.barbutton[i].fg_color = cfg_getint(cfgtmp2, "fg_color");
          conf.barbutton[i].bg_color = cfg_getint(cfgtmp2, "bg_color");
          conf.barbutton[i].x = cfg_getint(cfgtmp2, "x");
     }
     cfg_free(cfg);
}


