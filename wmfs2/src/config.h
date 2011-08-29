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

static const struct { char *name; void (*func)(Uicb cmd); } uicb_list[] =
{
     /* Sys */
     { "spawn",  uicb_spawn },
     { "quit",   uicb_quit },
     { "reload", uicb_reload },

     /* Tag */
     { "tag_set",  uicb_tag_set },
     { "tag_next", uicb_tag_next },
     { "tag_prev", uicb_tag_prev },
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

void config_init(void);

#endif /* CONFIG_H */
