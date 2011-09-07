/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef FRAME_H
#define FRAME_H

#include "wmfs.h"

struct Frame *frame_new(struct Tag *t);
void frame_free(struct Tag *t);
void frame_update(struct Frame *f);

#endif /* FRAME_H */
