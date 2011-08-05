/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef EVENT_H
#define EVENT_H

#define MAX_EV 256

#define HANDLE_EVENT(e) event_handle[(e)->type](e);

void event_init(void);

void (*event_handle)(XEvent*)[MAX_EV];

#endif /* EVENT_H */
