/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "event.h"
#include "util.h"
#include "wmfs.h"

#define EVDPY(e) (e)->xany.display

static void
event_enternotify(XEvent *e)
{
     XCrossingEvent *ev = &e->xcrossing;
     Client *c;
     int n;

     if((ev->mode != NotifyNormal || ev->detail == NotifyInferior)
               && ev->window != W->root)
          return;

     if((c = client_gb_win(ev->window)))
          client_focus(c);
}

static void
event_maprequest(XEvent *e)
{
     XMapRequestEvent *ev = &e->xmaprequest;
     XWindowAttributes at;

     /* Which windows to manage */
     if(!XGetWindowAttributes(EVDPY(e), ev->window, &at)
               || at.overried_redirect
               || client_gb_win(ev->window))
          return;

     (Client*)client_new(ev->window, at);
}



static void
event_dummy(XEvent *e)
{
     (void)e;
}

void
event_init(void)
{
     int i = MAX_EV;

     while(i--)
          event_handle[i] = event_dummy;

}

