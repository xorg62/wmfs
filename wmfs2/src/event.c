/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "event.h"
#include "util.h"
#include "wmfs.h"
#include "client.h"

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
event_configureevent(XEvent *e)
{
     XConfigureRequestEvent *ev = &e->xconfigurerequest;
     XWindowChanges wc;
     Client *c;

     if((c = client_gb_win(ev->window)))
     {
          if(ev->value_mask & CWX)
               c->geo.x = ev->x;
          if(ev->value_mask & CWY)
               c->geo.y = ev->y;
          if(ev->value_mask & CWWidth)
               c->geo.w = ev->width;
          if(ev->value_mask & CWHeight)
               c->geo.h = ev->height;

          client_configure(c);
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
     Client *c;

     if((c = client_gb_win(ev->window)))
          client_remove(c);
}

static void
event_focusin(XEvent *e)
{
     Client *c;

     if((c = W->screen->seltag->sel) && e->xfocus.window != c->win)
          client_focus(c);
}

static void
event_maprequest(XEvent *e)
{
     XMapRequestEvent *ev = &e->xmaprequest;
     XWindowAttributes at;

     /* Which windows to manage */
     if(!XGetWindowAttributes(EVDPY(e), ev->window, &at)
               || at.override_redirect)
          return;

     if(!client_gb_win(ev->window))
          (Client*)client_new(ev->window, &at);
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
     Client *c;

     if(ev->state == PropertyDelete)
          return;

     if((c = client_gb_win(ev->window)))
     {
          switch(ev->atom)
          {
               case XA_WM_TRANSIENT_FOR:
                    break;
               case XA_WM_NORMAL_HINTS:
                    /* client_get_size_hints(c); */
                    break;
               case XA_WM_HINTS:
                    /*
                    XWMHints *h;

                    if((h = XGetWMHints(EVDPY, c->win)) && (h->flags & XUrgencyHint) && c != sel)
                    {
                         client_urgent(c, True);
                         XFree(h);
                    }
                     */
                    break;
               default:
                    if(ev->atom == XA_WM_NAME /* || ev->atom == _NET_WM_NAME */)
                         client_get_name(c);
                    break;
          }
     }
}

static void
event_unmapnotify(XEvent *e)
{
     XUnmapEvent *ev = &e->xunmap;
     Client *c;

     if((c = client_gb_win(ev->window)) && ev->send_event)
          client_remove(c);
}

static void
event_motionnotify(XEvent *e)
{
     /*
        XMotionEvent *ev = &e->xmotion;
        Client *c;


      * Option follow mouvement
        if((c = client_gb_win(ev->subwindow)) && c != c->tag->sel)
             client_focus(c);
      */
}

static void
event_keypress(XEvent *e)
{
     XKeyPressedEvent *ev = &e->xkey;
     KeySym keysym;
     Keybind *k;
     Flags m = ~(W->numlockmask | LockMask);

     keysym = XKeycodeToKeysym(EVDPY(e), (KeyCode)ev->keycode, 0);

     SLIST_FOREACH(k, &W->h.keybind, next)
          if(k->keysym == keysym && (k->mod & m) == (ev->state & m))
               if(k->func)
                    k->func(k->cmd);
}

static void
event_expose(XEvent *e)
{
     /*
      *  XExposeEvent *ev = &e->xexpose;
      */

}

static void
event_dummy(XEvent *e)
{
     printf("%d\n", e->type);
     (void)e;
}

void
event_init(void)
{
     int i = MAX_EV;

     while(i--)
          event_handle[i] = event_dummy;

     /*event_handle[ButtonPress]      = event_buttonpress;*/
     /*event_handle[ClientMessage]    = event_clientmessageevent;*/
     event_handle[ConfigureRequest] = event_configureevent;
     event_handle[DestroyNotify]    = event_destroynotify;
     event_handle[EnterNotify]      = event_enternotify;
     event_handle[Expose]           = event_expose;
     event_handle[FocusIn]          = event_focusin;
     event_handle[KeyPress]         = event_keypress;
     /*event_handle[MapNotify]        = event_mapnotify;*/
     event_handle[MapRequest]       = event_maprequest;
     event_handle[MappingNotify]    = event_mappingnotify;
     event_handle[MotionNotify]     = event_motionnotify;
     event_handle[PropertyNotify]   = event_propertynotify;
     /*event_handle[ReparentNotify]   = event_reparentnotify;*/
     /*event_handle[SelectionClear]   = event_selectionclearevent;*/
     event_handle[UnmapNotify]      = event_unmapnotify;
}

