/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include "wmfs.h"

void systray_acquire(void);
void systray_add(Window win);
void systray_del(struct _systray *s);
void systray_state(struct _systray *s);
void systray_freeicons(void);
struct _systray *systray_find(Window win);
int systray_get_width(void);
void systray_update(void);

#endif /* SYSTRAY_H */
