/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "tag.h"

#ifndef SCREEN_H
#define SCREEN_H

typedef struct Screen
{
     Geo geo;
     Tag *seltag;
     SLIST_HEAD(, Tag) tags;
     SLIST_ENTRY(Screen) next;
} Screen;

void screen_init(void);
void screen_free(void);

#endif /* SCREEN_H */
