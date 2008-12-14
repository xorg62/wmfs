/*
*      event.c
*      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
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

/** ButtonPress handle event
 * \param ev XButtonEvent pointer
*/
void
buttonpress(XButtonEvent *ev)
{
     Client *c;
     int i;

     screen_get_sel();

     /* Titlebar */
     if((c = client_gb_titlebar(ev->window)))
          for(i = 0; i < conf.titlebar.nmouse; ++i)
                if(ev->button == conf.titlebar.mouse[i].button)
                    if(conf.titlebar.mouse[i].func)
                         conf.titlebar.mouse[i].func(conf.titlebar.mouse[i].cmd);

     /* Frame Resize Area */
     if((c = client_gb_resize(ev->window)))
          mouse_resize(c);

     /* Client */
     if((c = client_gb_win(ev->window)))
          for(i = 0; i < conf.client.nmouse; ++i)
               if(ev->button == conf.client.mouse[i].button)
                    if(conf.client.mouse[i].func)
                         conf.client.mouse[i].func(conf.client.mouse[i].cmd);

     /* Root */
     if(ev->window == ROOT)
          for(i = 0; i < conf.root.nmouse; ++i)
               if(conf.root.mouse[i].tag == seltag[conf.root.mouse[i].screen]
                  || conf.root.mouse[i].tag < 0)
                    if(ev->button == conf.root.mouse[i].button)
                         if(conf.root.mouse[i].func)
                              conf.root.mouse[i].func(conf.root.mouse[i].cmd);

     /* Tags */
     for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
          if(ev->window == infobar[selscreen].tags[i]->win)
               switch(ev->button)
               {
                case Button1: tag_set(i);                     break;
                case Button3: tag_transfert(sel, i);          break;
                case Button4: tag_set(seltag[selscreen] + 1); break;
                case Button5: tag_set(seltag[selscreen] - 1); break;
               }

     /* Layout button */
     if(ev->window == infobar[selscreen].layout_button->win)
          switch(ev->button)
          {
           case Button1: case Button4: layoutswitch(True);  break;
           case Button3: case Button5: layoutswitch(False); break;
          }

     return;
}

/* ClientMessage handle event
 *\param ev XClientMessageEvent pointer
*/
void
clientmessageevent(XClientMessageEvent *ev)
{
     Client *c;
     int i, mess_t = 0;

     if(ev->format != 32)
          return;

     for(i = 0; i < net_last; ++i)
          if(net_atom[i] == ev->message_type)
               mess_t = i;

     if(ev->window == ROOT)
     {
          /* Manage _NET_CURRENT_DESKTOP */
          if(mess_t == net_current_desktop
             && ev->data.l[0] >= 0
             && ev->data.l[0] < conf.ntag[selscreen])
               tag_set((int)(ev->data.l[0] + 1));

          /* Manage _NET_ACTIVE_WINDOW */
          else if(mess_t == net_active_window)
               if((c = client_gb_win((Window)ev->data.l[0])))
                    client_focus(c);

     }

     /* Manage _NET_WM_STATE */
     if(mess_t == net_wm_state)
          if((c = client_gb_win(ev->window)))
               ewmh_manage_net_wm_state(ev->data.l, c);
     /* Manage _NET_CLOSE_WINDOW */
     if(mess_t == net_close_window)
          if((c = client_gb_win(ev->window)))
               client_kill(c);

     return;
}

/** ConfigureRequest & ConfigureNotify handle event
 * \param ev XEvent pointer
*/
void
configureevent(XEvent *ev)
{
     XWindowChanges wc;
     XWindowAttributes win_at, ev_at;
     XRectangle geo = { 0 };
     Client *c;

     /* Check part */
     if((c = client_gb_win(ev->xconfigurerequest.window))
        || (c = client_gb_win(ev->xconfigure.window)))
     {
          CHECK(!c->tile);
          CHECK(!c->lmax);
     }

     /* Configure Request Part {{{ */
     wc.x = ev->xconfigurerequest.x;
     wc.y = ev->xconfigurerequest.y;
     wc.width = ev->xconfigurerequest.width;
     wc.height = ev->xconfigurerequest.height;
     wc.border_width = ev->xconfigurerequest.border_width;
     wc.sibling = ev->xconfigurerequest.above;
     wc.stack_mode = ev->xconfigurerequest.detail;
     XConfigureWindow(dpy, ev->xconfigurerequest.window,
                      ev->xconfigurerequest.value_mask, &wc);
     /* }}} */

     /* Configure Notify Part  {{{*/
     if((c = client_gb_win(ev->xconfigure.window)))
     {
          XGetWindowAttributes(dpy, ev->xconfigure.window, &win_at);
          XGetWindowAttributes(dpy, ev->xconfigure.event, &ev_at);

          /* Frame config */
          if(win_at.width != ev_at.width
             || win_at.height != ev_at.height)
          {
               c->geo.width = geo.width = ev->xconfigure.width;
               c->geo.height = geo.height = ev->xconfigure.height;
               frame_moveresize(c, geo);
          }

          /* Win config (re-adjust it with the frame) */
          if(ev->xconfigure.x != BORDH
             || ev->xconfigure.y != BORDH + TBARH)
               XMoveWindow(dpy, ev->xconfigure.window, BORDH, BORDH + TBARH);
     }
     /* }}} */


     return;
}

/** DestroyNotify handle event
 * \param ev XDestroyWindowEvent pointer
*/
void
destroynotify(XDestroyWindowEvent *ev)
{
     Client *c;

     if((c = client_gb_win(ev->window)))
     {
          client_unmanage(c);
          XSetErrorHandler(errorhandler);
     }

     return;
}

/** EnterNotify handle event
 * \param ev XCrossingEvent pointer
*/
void
enternotify(XCrossingEvent *ev)
{
     Client *c;

     if(ev->mode != NotifyNormal
        || ev->detail == NotifyInferior)
          return;
     if((c = client_gb_win(ev->window))
        || (c = client_gb_frame(ev->window))
        || (c = client_gb_titlebar(ev->window)))
          client_focus(c);
     else
          client_focus(NULL);

     return;
}

/** ExposeEvent handle event
 * \param ev XExposeEvent pointer
*/
void
expose(XExposeEvent *ev)
{
     Client *c;
     int i, sc;

     /* InfoBar member */
     for(sc = 0; sc < screen_count(); ++sc)
     {
          if(ev->window == infobar[sc].bar->win)
               barwin_refresh(infobar[sc].bar);
          if(ev->window == infobar[sc].layout_button->win)
               barwin_refresh(infobar[sc].layout_button);
          for(i = 1; i < conf.ntag[sc] + 1; ++i)
               if(ev->window == infobar[sc].tags[i]->win)
                    barwin_refresh(infobar[sc].tags[i]);
     }

     /* Client frame */
     if((c = client_gb_titlebar(ev->window)))
          frame_update(c);

     return;
}

/** FocusChange handle event
 * \param ev XFocusChangeEvent pointer
 * \return
*/
void
focusin(XFocusChangeEvent *ev)
{
     if(sel && ev->window != sel->win)
          client_focus(sel);

     return;
}

/** Key grabbing function
*/
void
grabkeys(void)
{
     uint i;
     KeyCode code;

     XUngrabKey(dpy, AnyKey, AnyModifier, ROOT);
     for(i = 0; i < conf.nkeybind; ++i)
     {
          code = XKeysymToKeycode(dpy, keys[i].keysym);
          XGrabKey(dpy, code, keys[i].mod, ROOT, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask, ROOT, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|numlockmask, ROOT, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask|numlockmask, ROOT, True, GrabModeAsync, GrabModeAsync);
     }

     return;
}

/** KeyPress handle event
 * \param ev XKeyPressedEvent pointer
*/
void
keypress(XKeyPressedEvent *ev)
{
     uint i;
     KeySym keysym;

     keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
     for(i = 0; i < conf.nkeybind; ++i)
          if(keysym == keys[i].keysym
             && (keys[i].mod & ~(numlockmask | LockMask))
             == (ev->state & ~(numlockmask | LockMask))
             && keys[i].func)
               keys[i].func(keys[i].cmd);

     return;
}

/** MapNotify handle event
 * \param ev XMappingEvent pointer
*/
void
mappingnotify(XMappingEvent *ev)
{
     if(ev->request == MappingKeyboard)
          grabkeys();

     return;
}

/** MapRequest handle event
 * \param ev XMapRequestEvent pointer
*/
void
maprequest(XMapRequestEvent *ev)
{
     XWindowAttributes at;
     Client *c;

     CHECK(XGetWindowAttributes(dpy, ev->window, &at));
     CHECK(!at.override_redirect);
     if(!(c = client_gb_win(ev->window)))
          client_manage(ev->window, &at);

     return;
}

/** PropertyNotify handle event
 * \param ev XPropertyEvent pointer
*/
void
propertynotify(XPropertyEvent *ev)
{
     Client *c;
     Window trans;

     if(ev->state == PropertyDelete)
          return;
     if((c = client_gb_win(ev->window)))
     {
          switch(ev->atom)
          {
          default: break;
          case XA_WM_TRANSIENT_FOR:
               XGetTransientForHint(dpy, c->win, &trans);
               if((c->tile || c->max) && (c->hint = (client_gb_win(trans) != NULL)))
                    arrange();
               break;
          case XA_WM_NORMAL_HINTS:
               client_size_hints(c);
               break;
          }
          if(ev->atom == XA_WM_NAME
             || ev->atom == net_atom[net_wm_name])
               client_get_name(c);
     }

     return;
}

/** UnmapNotify handle event
 * \param ev XUnmapEvent pointer
 */
void
unmapnotify(XUnmapEvent *ev)
{
     Client *c;

     if((c = client_gb_win(ev->window))
        && ev->send_event
        && !c->hide)
     {
          client_unmanage(c);
          XSetErrorHandler(errorhandler);
     }

     return;
}


/** Event handle function: execute every function
 * handle by event
 * \param ev Event
 */
void
getevent(XEvent ev)
{
     int st;

     switch (ev.type)
     {
      case ButtonPress:      buttonpress(&ev.xbutton);          break;
      case ClientMessage:    clientmessageevent(&ev.xclient);   break;
      case ConfigureRequest: configureevent(&ev);               break;
      case DestroyNotify:    destroynotify(&ev.xdestroywindow); break;
      case EnterNotify:      enternotify(&ev.xcrossing);        break;
      case Expose:           expose(&ev.xexpose);               break;
      case FocusIn:          focusin(&ev.xfocus);               break;
      case KeyPress:         keypress(&ev.xkey);                break;
      case MapRequest:       maprequest(&ev.xmaprequest);       break;
      case MappingNotify:    mappingnotify(&ev.xmapping);       break;
      case PropertyNotify:   propertynotify(&ev.xproperty);     break;
      case UnmapNotify:      unmapnotify(&ev.xunmap);           break;
     }

     wait(&st);

     return;
}
