/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "systray.h"
#include "ewmh.h"
#include "infobar.h"

#define SYSTRAY_SPACING (2)

void
systray_acquire(void)
{
     Window w = 0;
     XSetWindowAttributes wattr =
     {
          .event_mask = ButtonPressMask | ExposureMask,
          .override_redirect = true,
          .background_pixmap = ParentRelative,
          .background_pixel  = W->systray.infobar->theme->bars.bg,
     };

     if(!(W->flags & WMFS_SYSTRAY) || W->systray.win)
          return;

     if(XGetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s]) != None)
     {
          warnx("Can't initialize system tray: owned by another process.");
          return;
     }

     SLIST_INIT(&W->systray.head);

     /* Init systray window */
     w = XCreateSimpleWindow(W->dpy, W->systray.barwin->win, 0, 0,
                             W->systray.barwin->geo.h, W->systray.barwin->geo.h, 0, 0, 0);

     XChangeWindowAttributes(W->dpy, w, CWEventMask | CWOverrideRedirect | CWBackPixel, &wattr);
     XSelectInput(W->dpy, w, KeyPressMask | ButtonPressMask);
     XMapRaised(W->dpy, w);

     XSetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s], w, CurrentTime);

     if(XGetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s]) != w)
     {
          warnl("System tray: can't get systray manager");
          systray_freeicons();
          return;
     }

     ewmh_send_message(W->root, W->root, "MANAGER", CurrentTime,
                       W->net_atom[net_system_tray_s], w, 0, 0);

     XSync(W->dpy, false);

     W->systray.win = w;
}

void
systray_add(Window win)
{
     struct _systray *s;

     if(!(W->flags & WMFS_SYSTRAY))
          return;

     s = xcalloc(1, sizeof(struct _systray));
     s->win = win;

     s->geo.h = W->systray.barwin->geo.h;
     s->geo.w = W->systray.barwin->geo.h + SYSTRAY_SPACING;

     ewmh_set_wm_state(s->win, WithdrawnState);
     XSelectInput(W->dpy, s->win, StructureNotifyMask | PropertyChangeMask| EnterWindowMask | FocusChangeMask);
     XReparentWindow(W->dpy, s->win, W->systray.win, 0, 0);

     ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime,
                       XEMBED_EMBEDDED_NOTIFY, 0, W->systray.win, 0);

     SLIST_INSERT_HEAD(&W->systray.head, s, next);

     W->systray.redim = true;
}

void
systray_del(struct _systray *s)
{
     if(!(W->flags & WMFS_SYSTRAY))
          return;

     SLIST_REMOVE(&W->systray.head, s, _systray, next);
     free(s);

     W->systray.redim = true;
}

void
systray_state(struct _systray *s)
{
     long flags;
     int code = 0;

     if(!(W->flags & WMFS_SYSTRAY) || !(flags = ewmh_get_xembed_state(s->win)))
          return;

     if(flags & XEMBED_MAPPED)
     {
          code = XEMBED_WINDOW_ACTIVATE;
          XMapRaised(W->dpy, s->win);
          ewmh_set_wm_state(s->win, NormalState);
     }
     else
     {
          code = XEMBED_WINDOW_DEACTIVATE;
          XUnmapWindow(W->dpy, s->win);
          ewmh_set_wm_state(s->win, WithdrawnState);
     }

     ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime, code, 0, 0, 0);
}

void
systray_freeicons(void)
{
     struct _systray *i;

     if(!(W->flags & WMFS_SYSTRAY))
          return;

     while(!SLIST_EMPTY(&W->systray.head))
     {
          i = SLIST_FIRST(&W->systray.head);
          SLIST_REMOVE_HEAD(&W->systray.head, next);

          XUnmapWindow(W->dpy, i->win);
          XReparentWindow(W->dpy, i->win, W->root, 0, 0);
          free(i);
     }

     XSetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s], None, CurrentTime);
     W->systray.barwin->geo.w = 0;
     infobar_elem_reinit(W->systray.infobar);
     XSync(W->dpy, false);
}

struct _systray*
systray_find(Window win)
{
     struct _systray *i;

     if(!(W->flags & WMFS_SYSTRAY))
          return NULL;

     SLIST_FOREACH(i, &W->systray.head, next)
          if(i->win == win)
               return i;

     return NULL;
}

int
systray_get_width(void)
{
     int w = 1;
     struct _systray *i;

     SLIST_FOREACH(i, &W->systray.head, next)
          w += i->geo.w + SYSTRAY_SPACING;

     return w;
}

void
systray_update(void)
{
     int x = 1;
     struct _systray *i;

     if(!(W->flags & WMFS_SYSTRAY))
          return;

     if(W->systray.redim)
     {
          W->systray.redim = false;
          infobar_elem_reinit(W->systray.infobar);
     }

     SLIST_FOREACH(i, &W->systray.head, next)
     {
          XMapWindow(W->dpy, i->win);
          XMoveResizeWindow(W->dpy, i->win, (i->geo.x = x), 0, i->geo.w, i->geo.h);

          x += i->geo.w + SYSTRAY_SPACING;
     }
}



