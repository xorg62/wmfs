/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef EVENT_H
#define EVENT_H

#include <X11/XKBlib.h>

#include "wmfs.h"

#define MAX_EV 256

#define KEYPRESS_MASK(m) (m & ~(W->numlockmask | LockMask))
#define EVENT_HANDLE(e) event_handle[(e)->type](e);

void event_init(void);

void (*event_handle[MAX_EV])(XEvent*);

#endif /* EVENT_H */
