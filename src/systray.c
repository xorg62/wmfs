/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "systray.h"

bool
systray_acquire(void)
{
     XSetWindowAttributes wattr =
          {
               .event_mask = ButtonPressMask | ExposureMask,
               .override_redirect = true,
               .background_pixmap = ParentRelative,
               .background_pixel  = 0
          };

     if(!(W->flags & WMFS_SYSTRAY) || W->systray.win)
          return false;

     if(XGetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s]) != None)
     {
          warnx("Can't initialize system tray: owned by another process.");
          return False;
     }

     /* Init traywin window */
     XChangeWindowAttributes(W->dpy, W->systray.win, CWEventMask | CWOverrideRedirect | CWBackPixel, &wattr);
     XSelectInput(W->dpy, W->systray.win, KeyPressMask | ButtonPressMask);
     XMapRaised(W->dpy, W->systray.win);
     XSetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s], W->systray.win, CurrentTime);

     if(XGetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s]) != W->systray.win)
     {
          systray_freeicons();
          warnx("System tray: can't get systray manager");

          return false;
     }

     ewmh_send_message(W->root, W->root, "MANAGER", CurrentTime,
                       W->net_atom[net_system_tray_s], W->systray.win, 0, 0);

     XSync(dpy, false);

     return true;
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
     s->geo.w = W->systray.barwin->geo.w + 2;

     ewmh_set_win_state(s->win, WithdrawnState);
     XSelectInput(W->dpy, s->win, StructureNotifyMask | PropertyChangeMask| EnterWindowMask | FocusChangeMask);
     XReparentWindow(W->dpy, s->win, W->systray.win, 0, 0);

     ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime,
                       XEMBED_EMBEDDED_NOTIFY, 0, W->systray,win, 0);

     SLIST_INSERT_HEAD(&W->systray.head, s, next);
}

void
systray_del(struct _systray *s)
{
     if(!(W->flags & WMFS_SYSTRAY))
          return;

     SLIST_REMOVE(&W->systray.head, s, _systray, next);
     free(s);
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
          ewmh_set_win_state(s->win, NormalState);
     }
     else
     {
          code = XEMBED_WINDOW_DEACTIVATE;
          XUnmapWindow(W->dpy, s->win);
          ewmh_set_win_state(s->win, WithdrawnState);
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

          XUnmapWindow(dpy, i->win);
          XReparentWindow(dpy, i->win, W->root, 0, 0);
          free(i);
     }

     XSetSelectionOwner(W->dpy, W->net_atom[net_system_tray_s], None, CurrentTime);
     XDestroyWindow(W->dpy, W->systray.win);
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
