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

     /* Frame & titlebar */
     if((c = frame_get_titlebar(ev.xbutton.window)))
          for(i = 0; i < conf.titlebar.nmouse; ++i)
                if(ev.xbutton.button == conf.titlebar.mouse[i].button)
                    if(conf.titlebar.mouse[i].func)
                         conf.titlebar.mouse[i].func(conf.titlebar.mouse[i].cmd);

     /* Frame Resize Area */
     if((c = frame_get_resize(ev.xbutton.window)))
          mouse_resize(c);

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
     {
          CHECK(!c->tile);
          CHECK(!c->lmax);
     }
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
     {
          client_moveresize(c, geo, True);
          XReparentWindow(dpy, c->win, c->frame,
                          conf.client.borderheight,
                          conf.titlebar.height + conf.client.borderheight);
     }
     XSync(dpy, False);

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
        || (c = frame_get(ev.xcrossing.window))
        || (c = frame_get_titlebar(ev.xcrossing.window))
        || (c = frame_get_resize(ev.xcrossing.window)))
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

     for(c = clients; c; c = c->next)
          if(ev.xexpose.window == c->titlebar)
               frame_update(c);

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

     CHECK(XGetWindowAttributes(dpy, ev.xmaprequest.window, &at));
     CHECK(!at.override_redirect);
     if(!client_get(ev.xmaprequest.window))
          client_manage(ev.xmaprequest.window, &at);

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
               client_get_name(c);
     }

     return;
}


void
unmapnotify(XEvent ev)
{
     Client *c;

     if((c = client_get(event.xunmap.window))
        && ev.xunmap.event == root
        && ev.xunmap.send_event
        && getwinstate(c->win) == NormalState
        && !c->hide)
          client_unmanage(c);

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
           //case UnmapNotify:       unmapnotify(event);       break;
     }

     return;
}
