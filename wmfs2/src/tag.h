/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef TAG_H
#define TAG_H

#include "wmfs.h"
#include "screen.h"
#include "client.h"

typedef struct Tag
{
     char *name;
     Screen *screen;
     Flags flags;
     Client *sel;
     SLIST_ENTRY(Tag) next;
} Tag;

#endif /* TAG_H */
