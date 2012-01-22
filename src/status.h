/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef STATUS_H
#define STATUS_H

#include "wmfs.h"

struct status_ctx status_new_ctx(struct barwin *b, struct theme *t);
void status_render(struct status_ctx *ctx);
void status_manage(struct status_ctx *ctx);
void uicb_status(Uicb cmd);

#endif /* STATUS_H */
