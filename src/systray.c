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

#define TRAY_DWIDTH   (infobar[conf.systray.screen].bar->geo.height + conf.systray.spacing)

bool
systray_acquire(void)
{
     XSetWindowAttributes wattr;

     if(!conf.systray.active || traywin)
          return False;

     if(XGetSelectionOwner(dpy, net_atom[net_system_tray_s]) != None)
     {
          warnx("Can't initialize system tray: owned by another process");
          return False;
     }

     /* Init traywin window */
     wattr.event_mask        = ButtonPressMask | ExposureMask;
     wattr.override_redirect = True;
     wattr.background_pixmap = ParentRelative;
     wattr.background_pixel  = conf.colors.bar;

     traywin = XCreateSimpleWindow(dpy, infobar[conf.systray.screen].bar->win, -1, -1, 1, 1, 0, 0, conf.colors.bar);

     XChangeWindowAttributes(dpy, traywin, CWEventMask | CWOverrideRedirect | CWBackPixel, &wattr);
     XSelectInput(dpy, traywin, KeyPressMask | ButtonPressMask);

     XMapRaised(dpy, traywin);

     XSetSelectionOwner(dpy, net_atom[net_system_tray_s], traywin, CurrentTime);

     if(XGetSelectionOwner(dpy, net_atom[net_system_tray_s]) != traywin)
     {
          systray_freeicons();
          warnx("System tray: can't get systray manager");
          return False;
     }

     ewmh_send_message(ROOT, ROOT, "MANAGER", CurrentTime, net_atom[net_system_tray_s], traywin, 0, 0);

     XSync(dpy, False);

     return True;
}

void
systray_add(Window win)
{
     Systray *s;

     if(!conf.systray.active)
          return;

     s = xcalloc(1, sizeof(Systray));
     s->win = win;

     s->geo.height = infobar[conf.systray.screen].bar->geo.height;
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

     if(!conf.systray.active)
          return;

     for(ss = &trayicons; *ss && *ss != s; ss = &(*ss)->next);
     *ss = s->next;

     free(s);

     return;
}

void
systray_state(Systray *s)
{
     long flags;
     int code = 0;

     if(!(flags = ewmh_get_xembed_state(s->win)) || !conf.systray.active)
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
	Systray *i;

     if(!conf.systray.active)
          return;

     for(i = trayicons; i; i = i->next)
     {
          XUnmapWindow(dpy, i->win);
		XReparentWindow(dpy, i->win, ROOT, 0, 0);
          free(i);
     }

     XSetSelectionOwner(dpy, net_atom[net_system_tray_s], None, CurrentTime);
     XDestroyWindow(dpy, traywin);

     XSync(dpy, 0);

     return;
}

Systray*
systray_find(Window win)
{
	Systray *i;

     if(!conf.systray.active)
          return NULL;

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

     if(!conf.systray.active)
          return 0;

     for(i = trayicons; i; i = i->next)
          w += i->geo.width + conf.systray.spacing + 1;

     return w;
}

void
systray_update(void)
{
     Systray *i;
     int x = 1;

     if(!conf.systray.active)
          return;

     if(!trayicons)
     {
          XMoveResizeWindow(dpy, traywin, infobar[conf.systray.screen].bar->geo.width - 1, 0, 1, 1);
          return;
     }

     for(i = trayicons; i; i = i->next)
     {
          XMapWindow(dpy, i->win);

          XMoveResizeWindow(dpy, i->win, (i->geo.x = x), 0, i->geo.width, i->geo.height);

          x += i->geo.width + conf.systray.spacing;
     }

     XMoveResizeWindow(dpy, traywin, infobar[conf.systray.screen].bar->geo.width - x,
               0, x, infobar[conf.systray.screen].bar->geo.height);

     return;
}
