/*
*      systray.c
*      Copyright © 2008, 2009, 2010 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

#define TRAY_SPACING  (3)
#define TRAY_DWIDTH   (infobar[0].bar->geo.height + TRAY_SPACING)

Bool
systray_acquire(void)
{
     char systray_atom[32];

     snprintf(systray_atom, sizeof(systray_atom), "_NET_SYSTEM_TRAY_S%u", SCREEN);
     trayatom = XInternAtom(dpy, systray_atom, False);

     XSetSelectionOwner(dpy, ATOM(systray_atom), traywin, CurrentTime);

     if(XGetSelectionOwner(dpy, trayatom) != traywin)
          return False;

     ewmh_send_message(ROOT, ROOT, "MANAGER", CurrentTime, trayatom, traywin, 0, 0);

     return True;
}

void
systray_init(void)
{
     XSetWindowAttributes wattr;

     /* Init traywin window */
     wattr.event_mask        = ButtonPressMask|ExposureMask;
     wattr.override_redirect = True;
     wattr.background_pixel  = conf.colors.bar;

     traywin = XCreateSimpleWindow(dpy, infobar[0].bar->win, 0, 0, 1, 1, 0, 0, conf.colors.bar);

     XChangeWindowAttributes(dpy, traywin, CWEventMask | CWOverrideRedirect | CWBackPixel, &wattr);
     XSelectInput(dpy, traywin, KeyPressMask | ButtonPressMask);

     XMapRaised(dpy, traywin);

     /* Select tray */
     if(!systray_acquire())
          warnx("Can't initialize system tray: owned by another process");

     return;
}

void
systray_kill(void)
{
     XSetSelectionOwner(dpy, trayatom, None, CurrentTime);
     XUnmapWindow(dpy, traywin);

     return;
}

void
systray_add(Window win)
{
     Systray *s = emalloc(1, sizeof(Systray));

     s->win = win;

     s->geo.height = infobar[0].bar->geo.height;
     s->geo.width  = TRAY_DWIDTH;

     setwinstate(s->win, WithdrawnState);
     XSelectInput(dpy, s->win, StructureNotifyMask | PropertyChangeMask| EnterWindowMask | FocusChangeMask);
     XReparentWindow(dpy, s->win, traywin, 0, 0);

     ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, traywin, 0);

     /* Attach */
	if(trayicons)
		trayicons->prev = s;

     s->next = trayicons;
     trayicons = s;

     return;
}

void
systray_del(Systray *s)
{
     Systray **ss;

     for(ss = &trayicons; *ss && *ss != s; ss = &(*ss)->next);
     *ss = s->next;

     IFREE(s);

     return;
}

void
systray_configure(Systray *s)
{
     long d = 0;
     XSizeHints *sh = NULL;

     if(!(sh = XAllocSizeHints()))
          return;

     XGetWMNormalHints(dpy, s->win, sh, &d);

     /* TODO: Improve this.. */
     if(d > 0)
          if(sh->flags & (USSize|PSize))
               s->geo.width = sh->width;

     XFree(sh);

     return;
}

void
systray_state(Systray *s)
{
     long flags;
     int code = 0;

     if(!(flags = ewmh_get_xembed_state(s->win)))
          return;

     if(flags & XEMBED_MAPPED)
     {
          code = XEMBED_WINDOW_ACTIVATE;
          XMapRaised(dpy, s->win);
          setwinstate(s->win, NormalState);
     }
     else
     {
          code = XEMBED_WINDOW_DEACTIVATE;
          XUnmapWindow(dpy, s->win);
          setwinstate(s->win, WithdrawnState);
     }

     ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime, code, 0, 0, 0);

     return;
}

void
systray_freeicons(void)
{
	Systray *i, *next;

     for(i = trayicons; i; i = next)
     {
		next = i->next;
		XReparentWindow(dpy, i->win, ROOT, 0, 0);
          IFREE(i);
     }

	XSync(dpy, 0);

     return;
}

Systray*
systray_find(Window win)
{
	Systray *i;

     for(i = trayicons; i; i = i->next)
          if(i->win == win)
               return i;

     return NULL;
}

int
systray_get_width(void)
{
     int w = 0;
     Systray *i;

     for(i = trayicons; i; i = i->next)
          w += i->geo.width + TRAY_SPACING + 1;

     return w;
}

void
systray_update(void)
{
     Systray *i;
     XWindowAttributes xa;
     int x = 1;

     if(!trayicons)
     {
          XMoveResizeWindow(dpy, traywin, infobar[0].bar->geo.width - 1, 0, 1, 1);
          return;
     }

     for(i = trayicons; i; i = i->next)
     {
          memset(&xa, 0, sizeof(xa));
		XGetWindowAttributes(dpy, i->win, &xa);

          XMapWindow(dpy, i->win);

          if(xa.width < (i->geo.width = TRAY_DWIDTH))
               i->geo.width = xa.width;

          if(xa.height < (i->geo.height = infobar[0].bar->geo.height))
               i->geo.height = xa.height;

          XMoveResizeWindow(dpy, i->win, (i->geo.x = x), 0, i->geo.width, i->geo.height);

          x += i->geo.width + TRAY_SPACING;
     }

     XMoveResizeWindow(dpy, traywin, infobar[0].bar->geo.width - x, 0, x, infobar[0].bar->geo.height);

     return;
}
