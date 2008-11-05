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

/* BUTTONPRESS */
void
buttonpress(XEvent ev)
{
     Client *c;
     int i;
     char s[6];

     /* Titlebar */
     if(conf.titlebar.exist)
          if((c = titlebar_get(ev.xbutton.window)))
               for(i = 0; i < conf.titlebar.nmouse; ++i)
                    if(ev.xbutton.button == conf.titlebar.mouse[i].button)
                         if(conf.titlebar.mouse[i].func)
                              conf.titlebar.mouse[i].func(conf.titlebar.mouse[i].cmd);

     /* Client */
     if((c = client_get(ev.xbutton.window)))
          for(i = 0; i < conf.client.nmouse; ++i)
               if(ev.xbutton.button == conf.client.mouse[i].button)
                    if(conf.client.mouse[i].func)
                         conf.client.mouse[i].func(conf.client.mouse[i].cmd);

     /* Root */
     if(ev.xbutton.window == root)
          for(i = 0; i < conf.root.nmouse; ++i)
               if(ev.xbutton.button == conf.root.mouse[i].button)
                    if(conf.root.mouse[i].func)
                         conf.root.mouse[i].func(conf.root.mouse[i].cmd);

     /* Bar */
     {
          if(ev.xbutton.window == infobar.bar->win)
          {
               /* Tag*/
               for(i = 0; i < conf.ntag + 1; ++i)
               {
                    if(ev.xbutton.x > taglen[i-1] - 3
                       && ev.xbutton.x < (taglen[i] - 3))
                    {
                         ITOA(s, i);
                         if(ev.xbutton.button == Button1)
                              uicb_tag(s);
                         if(ev.xbutton.button == Button3)
                              uicb_tagtransfert(s);
                    }
               }
               if(ev.xbutton.x < taglen[conf.ntag])
               {
                    if(ev.xbutton.button == Button4)
                         uicb_tag("+1");
                    if (ev.xbutton.button == Button5)
                         uicb_tag("-1");
               }
          }
          /* Layout */
          {
               if(ev.xbutton.window == infobar.layout_switch->win)
               {
                    if(ev.xbutton.button == Button1
                       || ev.xbutton.button == Button4)
                         layoutswitch(True);

                    if(ev.xbutton.button == Button3
                       || ev.xbutton.button == Button5)
                         layoutswitch(False);
               }

               if(ev.xbutton.window == infobar.layout_type_switch->win)
               {
                    if(ev.xbutton.button == Button1
                       || ev.xbutton.button == Button4)
                         layout_tile_switch(True);

                    if(ev.xbutton.button == Button3
                       || ev.xbutton.button == Button5)
                         layout_tile_switch(False);
               }
          }
     }

     return;
}

/* CONFIGUREREQUEST */
void
configurerequest(XEvent ev)
{
     Client *c;
     XWindowChanges wc;
     XRectangle geo;
     if((c = client_get(ev.xconfigurerequest.window)))
          if(c->tile || c->lmax)
               return;
     geo.x = wc.x = ev.xconfigurerequest.x;
     geo.y = wc.y = ev.xconfigurerequest.y;
     geo.width = wc.width = ev.xconfigurerequest.width;
     geo.height = wc.height = ev.xconfigurerequest.height;
     wc.border_width = ev.xconfigurerequest.border_width;
     wc.sibling = ev.xconfigurerequest.above;
     wc.stack_mode = ev.xconfigurerequest.detail;
     XConfigureWindow(dpy, ev.xconfigurerequest.window,
                      ev.xconfigurerequest.value_mask, &wc);
     if((c = client_get(ev.xconfigurerequest.window)))
          if(wc.y < mw && wc.x < mh)
               client_moveresize(c, geo, True);

     return;
}

/* DESTROYNOTIFY */
void
destroynotify(XEvent ev)
{
     Client *c;
     if((c = client_get(ev.xdestroywindow.window)))
          client_unmanage(c);

     return;
}

/* ENTERNOTIFY */
void
enternotify(XEvent ev)
{
     Client *c;

     if(ev.xcrossing.mode != NotifyNormal
        || ev.xcrossing.detail == NotifyInferior)
          return;
     if((c = client_get(ev.xcrossing.window))
        || (c = titlebar_get(ev.xcrossing.window)))
          client_focus(c);
     else
          client_focus(NULL);

     return;
}

/* EXPOSE */
void
expose(XEvent ev)
{
     Client *c;

     if(ev.xexpose.count == 0
        && (ev.xexpose.window == infobar.bar->win))
          infobar_draw();

     if(conf.titlebar.exist)
          for(c = clients; c; c = c->next)
               if(ev.xexpose.window == c->tbar->win)
                    titlebar_update(c);

     return;
}

/* FOCUSIN */
void
focusin(XEvent ev)
{
     if(sel && ev.xfocus.window != sel->win)
          XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);

     return;
}

void
grabbuttons(Client *c, Bool focused)
{
     uint mod = conf.client.mod;

     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

     if(focused)
     {
          XGrabButton(dpy, Button1, mod, c->win, False, ButtonMask, GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, mod|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button1, mod|numlockmask, c->win, False, ButtonMask, GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, mod|LockMask|numlockmask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);

          XGrabButton(dpy, Button2, mod, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, mod|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, mod|numlockmask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, mod|LockMask|numlockmask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);

          XGrabButton(dpy, Button3, mod, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, mod|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, mod|numlockmask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, mod|LockMask|numlockmask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
     }
     else
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);

     return;
}

void
grabkeys(void)
{
     uint i;
     KeyCode code;

     XUngrabKey(dpy, AnyKey, AnyModifier, root);
     for(i = 0; i < conf.nkeybind; ++i)
     {
          code = XKeysymToKeycode(dpy, keys[i].keysym);
          XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|numlockmask, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask|numlockmask, root, True, GrabModeAsync, GrabModeAsync);
     }

     return;
}


/* KEYPRESS */
void
keypress(XEvent ev)
{
     uint i;
     KeySym keysym;

     keysym = XKeycodeToKeysym(dpy, (KeyCode)ev.xkey.keycode, 0);
     for(i = 0; i < conf.nkeybind; ++i)
          if(keysym == keys[i].keysym
             && (keys[i].mod & ~(numlockmask | LockMask)) ==
             (ev.xkey.state & ~(numlockmask | LockMask))
             && keys[i].func)
               keys[i].func(keys[i].cmd);

     return;
}

/* MAPPINGNOTIFY */
void
mapnotify(XEvent ev)
{
     if(ev.xmapping.request == MappingKeyboard)
          grabkeys();

     return;
}


/* MAPREQUEST */
void
maprequest(XEvent ev)
{
     XWindowAttributes at;

     if(!XGetWindowAttributes(dpy, ev.xmaprequest.window, &at))
          return;
     if(at.override_redirect)
          return;
     if(!client_get(ev.xmaprequest.window))
          client_manage(ev.xmaprequest.window, &at);

     return;
}

/* If the type is 0, this function will move, else,
 * this will resize */
void
mouseaction(Client *c, int x, int y, int type)
{
     int  ocx, ocy;
     int my = sgeo.y, mx = 0;
     double fy, fx, mwf;
     XRectangle geo;
     XEvent ev;

     if(c->max || (c->tile && !type) || c->lmax)
          return;

     ocx = c->geo.x;
     ocy = c->geo.y;
     if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[((type) ?CurResize:CurMove)], CurrentTime) != GrabSuccess)
          return;
     /* Warp pointer for resize */
     if(type && !c->tile)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width, c->geo.height);

     /* Warp pointer for mwfact resize */
     if(type && c->tile)
     {
          if(tags[seltag].layout.func == tile)
               mx = tags[seltag].mwfact * sgeo.width;
          else if(tags[seltag].layout.func == tile_left)
               mx = sgeo.width - (tags[seltag].mwfact * sgeo.width);
          else if(tags[seltag].layout.func == tile_top)
          {
               mx = event.xmotion.x;
               my = sgeo.height - (tags[seltag].mwfact * sgeo.height);
          }
          else if(tags[seltag].layout.func == tile_bottom)
          {
               mx = event.xmotion.x;
               my = tags[seltag].mwfact * sgeo.height;
          }
          XWarpPointer(dpy, None, root, 0, 0, 0, 0, mx, my);
     }

     for(;;)
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);
          if(ev.type == ButtonRelease)
          {
               if(type && !c->tile)
                    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->geo.width, c->geo.height);
               XUngrabPointer(dpy, CurrentTime);
               infobar_draw();
               return;
          }
          else if(ev.type == MotionNotify)
          {
               XSync(dpy, False);
               /* Resize */
               if(type && !c->tile)
               {
                    geo.x = c->geo.x; geo.y = c->geo.y;
                    geo.width = ((ev.xmotion.x - ocx < 1) ? 1 : ev.xmotion.x - ocx);
                    geo.height = ((ev.xmotion.y - ocy < 1) ? 1 : ev.xmotion.y - ocy);
                    client_moveresize(c, geo, True);
               }
               /* Set mwfact in tiled mode */
               else if(type && c->tile)
               {
                    fy = (round( ((ev.xmotion.y * 50) / sgeo.height)) ) / 50;
                    fx = (round( ((ev.xmotion.x * 50) / sgeo.width)) ) / 50;

                    if(tags[seltag].layout.func == tile)
                         mwf = fx;
                    else if(tags[seltag].layout.func == tile_left)
                         mwf = 1 - fx;
                    else if(tags[seltag].layout.func == tile_top)
                         mwf = 1 - fy;
                    else if(tags[seltag].layout.func == tile_bottom)
                         mwf = fy;

                    tags[seltag].mwfact =
                         (((0.01) > (mwf) ? (0.01) : (mwf) ) < 0.99)
                         ? ((0.01) > (mwf) ? (0.01): (mwf))
                         : 0.99;

                    arrange();
               }
               /* Move */
               else
               {
                    geo.x = (ocx + (ev.xmotion.x - x));
                    geo.y = (ocy + (ev.xmotion.y - y));
                    geo.width = c->geo.width;
                    geo.height = c->geo.height;
                    client_moveresize(c, geo, True);
               }
          }
          else if(ev.type == Expose)
               expose(ev);
     }

     return;
}

void
uicb_mousemove(uicb_t cmd)
{
     if(sel)
          mouseaction(sel, event.xbutton.x_root, event.xbutton.y_root, False);

     return;
}

void
uicb_resizemouse(uicb_t cmd)
{
     if(sel)
          mouseaction(sel, event.xbutton.x_root, event.xbutton.y_root, True);

     return;
}


/* PROPERTYNOTIFY */
void
propertynotify(XEvent ev)
{
     Client *c;
     Window trans;

     if(event.xproperty.state == PropertyDelete)
          return;
     if((c = client_get(event.xproperty.window)))
     {
          switch(event.xproperty.atom)
          {
          default: break;
          case XA_WM_TRANSIENT_FOR:
               XGetTransientForHint(dpy, c->win, &trans);
               if((c->tile || c->max) && (c->hint = (client_get(trans) != NULL)))
                    arrange();
               break;
          case XA_WM_NORMAL_HINTS:
               client_size_hints(c);
               break;
          }
          if(ev.xproperty.atom == XA_WM_NAME
             || ev.xproperty.atom == net_atom[NetWMName])
               titlebar_update(c);
     }

     return;
}

/* Handle */
void
getevent(void)
{
     switch (event.type)
     {
      case ButtonPress:       buttonpress(event);       break;
      case ConfigureRequest:  configurerequest(event);  break;
      case DestroyNotify:     destroynotify(event);     break;
      case EnterNotify:       enternotify(event);       break;
      case Expose:            expose(event);            break;
      case FocusIn:           focusin(event);           break;
      case KeyPress:          keypress(event);          break;
      case MapRequest:        maprequest(event);        break;
      case MappingNotify:     mapnotify(event);         break;
      case PropertyNotify:    propertynotify(event);    break;
     }

     return;
}
