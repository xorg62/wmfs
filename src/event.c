/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "event.h"
#include "ewmh.h"
#include "config.h"
#include "util.h"
#include "wmfs.h"
#include "client.h"
#include "barwin.h"
#include "screen.h"
#include "systray.h"
#include "infobar.h"

#define EVDPY(e) (e)->xany.display

#define MOUSE_DO_BIND(m)                                                     \
     if(m->button == ev->button)                                             \
          if(!m->use_area || (m->use_area && INAREA(ev->x, ev->y, m->area))) \
               if(m->func)                                                   \
                    m->func(m->cmd);
static void
event_buttonpress(XEvent *e)
{
     XButtonEvent *ev = &e->xbutton;
     struct mousebind *m;
     struct barwin *b;

     screen_update_sel();
     status_flush_surface();

     SLIST_FOREACH(b, &W->h.barwin, next)
          if(b->win == ev->window)
          {
               W->last_clicked_barwin = b;

               SLIST_FOREACH(m, &b->mousebinds, next)
                    MOUSE_DO_BIND(m);

               SLIST_FOREACH(m, &b->statusmousebinds, next)
                    MOUSE_DO_BIND(m);

               break;
          }
}

static void
event_enternotify(XEvent *e)
{
     XCrossingEvent *ev = &e->xcrossing;
     struct client *c;

     if((ev->mode != NotifyNormal
         || ev->detail == NotifyInferior)
               && ev->window != W->root)
          return;

     if(ev->window == W->systray.win || systray_find(ev->window))
          return;

     if((c = client_gb_win(ev->window))
        || (c = client_gb_frame(ev->window)))
     {
          if(c->flags & CLIENT_IGNORE_ENTER)
               c->flags ^= CLIENT_IGNORE_ENTER;
          else if(c->tag->flags & TAG_IGNORE_ENTER)
               c->tag->flags ^= TAG_IGNORE_ENTER;
          else if(c != W->client && !(c->flags & CLIENT_TABBED))
               client_focus(c);
     }
}

static void
event_clientmessageevent(XEvent *e)
{
     XClientMessageEvent *ev = &e->xclient;
     struct client *c;
     struct _systray *sy;
     int type = 0;

     while(type < net_last && W->net_atom[type] != ev->message_type)
          ++type;

     /*
      * Systray message
      * _NET_WM_SYSTRAY_TRAY_OPCODE
      */
     if(ev->window == W->systray.win && type == net_system_tray_opcode)
     {
          if(ev->data.l[1] == XEMBED_EMBEDDED_NOTIFY)
          {
               systray_add(ev->data.l[2]);
               systray_update();
          }
          else if(ev->data.l[1] == XEMBED_REQUEST_FOCUS)
          {
               if((sy = systray_find(ev->data.l[2])))
                    ewmh_send_message(sy->win, sy->win, "_XEMBED", XEMBED_FOCUS_IN,
                                      XEMBED_FOCUS_CURRENT, 0, 0, 0);
          }
     }
     else if(ev->window == W->root)
     {
          /* WMFS message */
          if(ev->data.l[4])
          {
               /* Manage _WMFS_FUNCTION && _WMFS_CMD */
               if(type == wmfs_function || type == wmfs_cmd)
               {
                    int d;
                    long unsigned int len;
                    unsigned char *ret = NULL, *ret_cmd = NULL;
                    void (*func)(Uicb);

                    if(XGetWindowProperty(EVDPY(e), W->root, W->net_atom[wmfs_function], 0, 65536,
                                          False, W->net_atom[utf8_string], (Atom*)&d, &d,
                                          (long unsigned int*)&d, (long unsigned int*)&d, &ret) == Success
                       && ret && ((func = uicb_name_func((char*)ret))))
                    {
                         if(XGetWindowProperty(EVDPY(e), W->root, W->net_atom[wmfs_cmd], 0, 65536,
                                               False, W->net_atom[utf8_string], (Atom*)&d, &d,
                                               &len, (long unsigned int*)&d, &ret_cmd) == Success
                            && len && ret_cmd)
                         {
                              func((Uicb)ret_cmd);
                              XFree(ret_cmd);
                         }
                         else
                              func(NULL);

                         XFree(ret);
                    }
               }
          }

          if(type == net_active_window)
               if((sy = systray_find(ev->data.l[0])))
                    XSetInputFocus(W->dpy, sy->win, RevertToNone, CurrentTime);
     }

     switch(type)
     {
          /* _NET_WM_STATE */
          case net_wm_state:
               if((c = client_gb_win(ev->window)))
                    ewmh_manage_state(ev->data.l, c);
               break;
          /* _NET_CLOSE_WINDOW */
          case net_close_window:
               if((c = client_gb_win(ev->window)))
                    client_close(c);
               break;
          /* _NET_WM_DESKTOP */
          case net_wm_desktop:
               break;
     }
}

static void
event_configureevent(XEvent *e)
{
     XConfigureRequestEvent *ev = &e->xconfigurerequest;
     XWindowChanges wc;
     struct client *c;

     if((c = client_gb_win(ev->window)))
     {
          if(c->flags & CLIENT_FREE)
          {
               if(ev->value_mask & CWX)
                    c->geo.x = ev->x;
               if(ev->value_mask & CWY)
                    c->geo.y = ev->y - c->tbarw - c->border - c->border;
               if(ev->value_mask & CWWidth)
                    c->geo.w = ev->width + c->border + c->border;
               if(ev->value_mask & CWHeight)
                    c->geo.h = ev->height + c->tbarw + c->border;

               client_moveresize(c, &c->geo);
          }
          else
          {
               if(ev->value_mask & CWWidth)
                    _fac_resize(c, Right, ev->width - c->wgeo.w);
               if(ev->value_mask & CWHeight)
                    _fac_resize(c, Bottom, ev->height - c->wgeo.h);

               client_apply_tgeo(c->tag);
          }
     }
     else
     {
          wc.x            = ev->x;
          wc.y            = ev->y;
          wc.width        = ev->width;
          wc.height       = ev->height;
          wc.border_width = ev->border_width;
          wc.sibling      = ev->above;
          wc.stack_mode   = ev->detail;

          XConfigureWindow(EVDPY(e), ev->window, ev->value_mask, &wc);
     }
}

static void
event_destroynotify(XEvent *e)
{
     XDestroyWindowEvent *ev = &e->xdestroywindow;
     struct client *c;
     struct _systray *s;

     if((c = client_gb_win(ev->window)))
          client_remove(c);
     else if((s = systray_find(ev->window)))
     {
          ewmh_set_wm_state(s->win, WithdrawnState);
          systray_del(s);
          systray_update();
     }
}

static void
event_focusin(XEvent *e)
{
     if(W->client
        && e->xfocus.window != W->client->win
        && e->xfocus.window != W->client->frame)
          client_focus(W->client);
}

static void
event_maprequest(XEvent *e)
{
     XMapRequestEvent *ev = &e->xmaprequest;
     XWindowAttributes at;
     struct _systray *s;

     /* Which windows to manage */
     if(!XGetWindowAttributes(EVDPY(e), ev->window, &at)
        || at.override_redirect)
          return;

     if(!client_gb_win(ev->window))
          client_new(ev->window, &at, false);
     else if((s = systray_find(ev->window)))
     {
          ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime,
                            XEMBED_WINDOW_ACTIVATE, 0, 0, 0);
          systray_update();
     }
}

static void
event_mappingnotify(XEvent *e)
{
     XMappingEvent *ev = &e->xmapping;
     XRefreshKeyboardMapping(ev);

     if(ev->request == MappingKeyboard)
          wmfs_grab_keys();
}

static void
event_propertynotify(XEvent *e)
{
     XPropertyEvent *ev = &e->xproperty;
     XWMHints *h;
     struct client *c;
     struct _systray *s;

     if(ev->state == PropertyDelete)
          return;

     if((c = client_gb_win(ev->window)))
     {
          switch(ev->atom)
          {
               case XA_WM_TRANSIENT_FOR:
                    break;

               case XA_WM_NORMAL_HINTS:
                    client_get_sizeh(c);
                    break;

               case XA_WM_HINTS:
                    if((h = XGetWMHints(EVDPY(e), c->win))
                       && (h->flags & XUrgencyHint)
                       && c->tag != W->screen->seltag)
                    {
                         c->tag->flags |= TAG_URGENT;
                         infobar_elem_screen_update(c->screen, ElemTag);
                         XFree(h);
                    }

                    break;

               default:
                    if(ev->atom == XA_WM_NAME || ev->atom == W->net_atom[net_wm_name])
                         client_get_name(c);
                    break;
          }
     }
     else if((s = systray_find(ev->window)))
     {
          systray_state(s);
          systray_update();
     }
}

static void
event_unmapnotify(XEvent *e)
{
     XUnmapEvent *ev = &e->xunmap;
     struct client *c;
     struct _systray *s;

     if((c = client_gb_win(ev->window))
        && ev->send_event
        && ev->event == W->root)
     {
          int d;
          unsigned char *ret = NULL;

          if(XGetWindowProperty(EVDPY(e), c->win, W->net_atom[wm_state], 0, 2,
                                False, W->net_atom[wm_state], (Atom*)&d, &d,
                                (long unsigned int*)&d, (long unsigned int*)&d, &ret) == Success)
               if(*ret == NormalState)
                    client_remove(c);
     }
     else if((s = systray_find(ev->window)))
     {
          systray_del(s);
          systray_update();
     }
}

static void
event_keypress(XEvent *e)
{
     XKeyPressedEvent *ev = &e->xkey;
     KeySym keysym = XKeycodeToKeysym(EVDPY(e), (KeyCode)ev->keycode, 0);
     struct keybind *k;

     screen_update_sel();
     status_flush_surface();

     SLIST_FOREACH(k, &W->h.keybind, next)
          if(k->keysym == keysym && KEYPRESS_MASK(k->mod) == KEYPRESS_MASK(ev->state))
               if(k->func)
                    k->func(k->cmd);
}

static void
event_expose(XEvent *e)
{
     XExposeEvent *ev = &e->xexpose;
     struct barwin *b;

     SLIST_FOREACH(b, &W->h.barwin, next)
          if(b->win == ev->window)
          {
               barwin_refresh(b);
               return;
          }
}

static void
event_mapnotify(XEvent *e)
{
     XMapEvent *ev = &e->xmap;
     struct client *c;
     struct _systray *s;

     if(ev->window != ev->event && !ev->send_event)
          return;

     if((c = client_gb_win(ev->window)))
          client_map(c);
     else if((s = systray_find(ev->window)))
     {
          ewmh_set_wm_state(s->win, NormalState);
          ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime,
                            XEMBED_WINDOW_ACTIVATE, 0, 0, 0);
     }
}

static void
event_selectionclearevent(XEvent *ev)
{
     /* Getting selection if lost it */
     if(ev->xselectionclear.window == W->systray.win)
          systray_acquire();

     systray_update();
}

static void
event_dummy(XEvent *e)
{
   /*  printf("%d\n", e->type);*/
     (void)e;
}

void
event_init(void)
{
     int i = MAX_EV;

     while(i--)
          event_handle[i] = event_dummy;

     event_handle[ButtonPress]      = event_buttonpress;
     event_handle[ClientMessage]    = event_clientmessageevent;
     event_handle[ConfigureRequest] = event_configureevent;
     event_handle[DestroyNotify]    = event_destroynotify;
     event_handle[EnterNotify]      = event_enternotify;
     event_handle[Expose]           = event_expose;
     event_handle[FocusIn]          = event_focusin;
     event_handle[KeyPress]         = event_keypress;
     event_handle[MapNotify]        = event_mapnotify;
     event_handle[MapRequest]       = event_maprequest;
     event_handle[MappingNotify]    = event_mappingnotify;
     event_handle[PropertyNotify]   = event_propertynotify;
     /*event_handle[ReparentNotify]   = event_reparentnotify;*/
     event_handle[SelectionClear]   = event_selectionclearevent;
     event_handle[UnmapNotify]      = event_unmapnotify;
}

