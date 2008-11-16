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
     char s[6];

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
     if(ev->window == root)
          for(i = 0; i < conf.root.nmouse; ++i)
               if(ev->button == conf.root.mouse[i].button)
                    if(conf.root.mouse[i].func)
                         conf.root.mouse[i].func(conf.root.mouse[i].cmd);

     /* Tag */
     for(i = 1; i < conf.ntag + 1; ++i)
     {
          ITOA(s, i);
          if(ev->window == infobar->tags[i]->win)
          {
               if(ev->button == Button1)
                    uicb_tag(s);
               if(ev->button == Button3)
                    uicb_tagtransfert(s);
               if(ev->button == Button4)
                    uicb_tag("+1");
               if (ev->button == Button5)
                    uicb_tag("-1");
          }
     }

     /* Layout button */
     if(ev->window == infobar->layout_button->win)
     {
          if(ev->button == Button1
             || ev->button == Button4)
               layoutswitch(True);
          if(ev->button == Button3
             || ev->button == Button5)
               layoutswitch(False);
     }

     return;
}

/** ConfigureRequest handle event
 * \param ev XConfigureRequestEvent pointer
*/
void
configurerequest(XConfigureRequestEvent *ev)
{
     Client *c;
     XWindowChanges wc;
     XRectangle geo;

     if((c = client_gb_win(ev->window)))
     {
          CHECK(!c->tile);
          CHECK(!c->lmax);

          if(ev->value_mask & CWX)
               geo.x = ev->x;
          if(ev->value_mask & CWY)
               geo.y = ev->y;
          if(ev->value_mask & CWWidth)
               geo.width = ev->width;
          if(ev->value_mask & CWHeight)
               geo.height = ev->height;
          if((ev->value_mask & (CWX | CWY))
             && !(ev->value_mask & (CWWidth | CWHeight)))
               client_configure(c);
          if(geo.x != c->geo.x
             || geo.y != c->geo.y
             || geo.width != c->geo.width
             || geo.height != c->geo.height)
          {
               geo.x += BORDH;
               geo.y += TBARH;
               if((geo.x < MAXW && geo.x > 0 - geo.width)
                  && (geo.y < MAXH && geo.y > 0 - geo.height))
                    client_moveresize(c, geo, True);
          }
          else
               client_configure(c);
     }
     else
     {
          wc.x = ev->x;
          wc.y = ev->y;
          wc.width = ev->width;
          wc.height = ev->height;
          wc.border_width = ev->border_width;
          wc.sibling = ev->above;
          wc.stack_mode = ev->detail;

          XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
      }

     XSync(dpy, False);

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
     int i;

     if(ev->count == 0
        && (ev->window == infobar->bar->win))
          bar_refresh(infobar->bar);

     for(i = 1; i < conf.ntag + 1; ++i)
          if(ev->window == infobar->tags[i]->win)
               bar_refresh(infobar->tags[i]);

     if(ev->window == infobar->layout_button->win)
          bar_refresh(infobar->layout_button);

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
          XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);

     return;
}

/** Key grabbing function
*/
void
grabkeys(void)
{
     uint i, j;
     KeyCode code;
     uint ml[] = {LockMask, numlockmask, scrolllockmask, numlockmask|scrolllockmask,
                  LockMask|scrolllockmask, LockMask|numlockmask, LockMask|numlockmask|scrolllockmask};

     XUngrabKey(dpy, AnyKey, AnyModifier, root);
     for(i = 0; i < conf.nkeybind; ++i)
     {
          code = XKeysymToKeycode(dpy, keys[i].keysym);
          for(j = 0; j < (sizeof ml / sizeof ml[0]); ++j)
               XGrabKey(dpy, code, keys[i].mod|ml[j], root, True, GrabModeAsync, GrabModeAsync);
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
             && (keys[i].mod & ~(numlockmask | LockMask)) ==
             (ev->state & ~(numlockmask | LockMask))
             && keys[i].func)
               keys[i].func(keys[i].cmd);

     return;
}

/** MapNotify handle event
 * \param ev XMappingEvent pointer
*/
void
mapnotify(XMappingEvent *ev)
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

     CHECK(XGetWindowAttributes(dpy, ev->window, &at));
     CHECK(!at.override_redirect);
     if(!client_gb_win(ev->window))
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
             || ev->atom == net_atom[NetWMName])
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
        && ev->event == root
        && ev->send_event
        && getwinstate(c->win) == NormalState
        && !c->hide)
     {
          XReparentWindow(dpy, c->win, root, 0, 0);
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
     switch (ev.type)
     {
      case ButtonPress:       buttonpress(&ev.xbutton);                 break;
      case ConfigureRequest:  configurerequest(&ev.xconfigurerequest);  break;
      case DestroyNotify:     destroynotify(&ev.xdestroywindow);        break;
      case EnterNotify:       enternotify(&ev.xcrossing);               break;
      case Expose:            expose(&ev.xexpose);                      break;
      case FocusIn:           focusin(&ev.xfocus);                      break;
      case KeyPress:          keypress(&ev.xkey);                       break;
      case MapRequest:        maprequest(&ev.xmaprequest);              break;
      case MappingNotify:     mapnotify(&ev.xmapping);                  break;
      case PropertyNotify:    propertynotify(&ev.xproperty);            break;
      case UnmapNotify:       unmapnotify(&ev.xunmap);                  break;
     }

     return;
}
