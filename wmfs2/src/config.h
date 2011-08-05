/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef CONFIG_H
#define CONFIG_H

#include "wmfs.h"

typedef struct Keybind
{
     unsigned int mod;
     KeySym keysym;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(Keybind) next;
} Keybind;

#endif /* CONFIG_H */
