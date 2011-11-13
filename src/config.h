/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef CONFIG_H
#define CONFIG_H

#include <string.h>
#include <X11/Xlib.h>

#include "wmfs.h"
#include "util.h"
#include "tag.h"
#include "client.h"

#define THEME_DEFAULT (SLIST_FIRST(&W->h.theme))

static const struct { char *name; void (*func)(Uicb cmd); } uicb_list[] =
{
     /* Sys */
     { "spawn",  uicb_spawn },
     { "quit",   uicb_quit },
     { "reload", uicb_reload },

     /* Tag */
     { "tag_set",  uicb_tag_set },
     { "tag",      uicb_tag_set_with_name },
     { "tag_next", uicb_tag_next },
     { "tag_prev", uicb_tag_prev },

     /* Layout */
     { "layout_vmirror",      uicb_layout_vmirror },
     { "layout_hmirror",      uicb_layout_hmirror },
     { "layout_rotate_left",  uicb_layout_rotate_left },
     { "layout_rotate_right", uicb_layout_rotate_right },
     { "layout_prev_set",     uicb_layout_prev_set },
     { "layout_next_set",     uicb_layout_next_set },

     /* Client */
     { "client_close",         uicb_client_close },
     { "client_resize_right",  uicb_client_resize_Right },
     { "client_resize_left",   uicb_client_resize_Left },
     { "client_resize_top",    uicb_client_resize_Top },
     { "client_resize_bottom", uicb_client_resize_Bottom },
     { "client_focus_right",   uicb_client_focus_Right },
     { "client_focus_left",    uicb_client_focus_Left },
     { "client_focus_top",     uicb_client_focus_Top },
     { "client_focus_bottom",  uicb_client_focus_Bottom },
     { "client_tab_right",     uicb_client_tab_Right },
     { "client_tab_left",      uicb_client_tab_Left },
     { "client_tab_top",       uicb_client_tab_Top },
     { "client_tab_bottom",    uicb_client_tab_Bottom },
     { "client_swap_right",    uicb_client_swap_Right },
     { "client_swap_left",     uicb_client_swap_Left },
     { "client_swap_top",      uicb_client_swap_Top },
     { "client_swap_bottom",   uicb_client_swap_Bottom },
     { "client_focus_next",    uicb_client_focus_next },
     { "client_focus_prev",    uicb_client_focus_prev },
     { "client_swap_next",     uicb_client_swapsel_next },
     { "client_swap_prev",     uicb_client_swapsel_prev },
     { NULL, NULL }
};

static inline void*
uicb_name_func(Uicb name)
{
     int i = 0;

     for(; uicb_list[i].func; ++i)
          if(!strcmp(name, uicb_list[i].name))
               return uicb_list[i].func;

     return NULL;
}

static const struct { const char *name; KeySym keysym; } key_list[] =
{
     {"Control", ControlMask },
     {"Shift",   ShiftMask },
     {"Lock",    LockMask },
     {"Alt",     Mod1Mask },
     {"Mod1",    Mod1Mask },
     {"Mod2",    Mod2Mask },
     {"Mod3",    Mod3Mask },
     {"Mod4",    Mod4Mask },
     {"Super",   Mod4Mask },
     {"Home",    Mod4Mask },
     {"Mod5",    Mod5Mask },
     {NULL,      NoSymbol }
};

static inline KeySym
modkey_keysym(const char *name)
{
     int i = 0;

     for(; key_list[i].name; ++i)
          if(!strcmp(name, key_list[i].name))
               return key_list[i].keysym;

     return NoSymbol;
}

static inline struct theme*
name_to_theme(const char *name)
{
     struct theme *t;

     SLIST_FOREACH(t, &W->h.theme, next)
          if(!strcmp(t->name, name))
               return t;

     return THEME_DEFAULT;
}


void config_init(void);

#endif /* CONFIG_H */
