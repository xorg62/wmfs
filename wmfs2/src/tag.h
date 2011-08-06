/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef TAG_H
#define TAG_H

#include "wmfs.h"

Tag *tag_new(Scr33n *s, char *name);
void tag_screen(Scr33n *s, Tag *t);
void tag_free(Scr33n *s);


#endif /* TAG_H */
