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

/* BUTTONPRESS
*  For make a code cleaner, i put
*  more {} (useless sometimes)
*  and BIG comment.*/
void
buttonpress(XEvent ev)
{
     Client *c;
     int i, j;
     char s[6];

     if(conf.ttbarheight)
     {
          /* ******** */
          /* TITLEBAR */
          /* ******** */
          {
               if((c = client_gettbar(ev.xbutton.window)))
               {
                    raiseclient(c);
                    /* BUTTON 1 */
                    {
                         if(ev.xbutton.button == Button1)
                              mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, False);
                    }
                    /* BUTTON 2 */
                    {
                         if(ev.xbutton.button == Button2)
                         {
                              if(tags[seltag].layout.func == tile)
                                   uicb_tile_switch(NULL);
                              else
                                   uicb_togglemax(NULL);
                         }
                    }
                    /* BUTTON 3 */
                    {
                         if(ev.xbutton.button == Button3)
                              mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, True);
                    }
               }

          }

     }

     /* ****** */
     /* CLIENT */
     /* ****** */
     {
          if((c = getclient(ev.xbutton.window)))
          {
               raiseclient(c);
               /* BUTTON 1 */
               {
                    if(ev.xbutton.button == Button1)
                         mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, False);
               }
               /* BUTTON 2 */
               {
                    if(ev.xbutton.button == Button2)
                    {
                         if(tags[seltag].layout.func == tile)
                              uicb_tile_switch(NULL);
                         else
                              uicb_togglemax(NULL);
                    }
               }
               /* BUTTON 3 */
               {
                    if(ev.xbutton.button == Button3)
                         mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, True);
               }
          }

     }

     /* *** */
     /* BAR */
     /* *** */
     {
          if(ev.xbutton.window == bar->win)
          {
               /* *** */
               /* TAG */
               /* *** */
               {

                    /* CLICK */
                    {
                         for(i = 0; i < conf.ntag + 1; ++i)
                         {
                              if(ev.xbutton.x > taglen[i-1] - 3
                                 && ev.xbutton.x < (taglen[i] - 3))
                              {
                                   ITOA(s, i);
                                   /* BUTTON 1 */
                                   {
                                        if(ev.xbutton.button == Button1)
                                             uicb_tag(s);
                                   }
                                   /* BUTTON 2 */
                                   {
                                        if(ev.xbutton.button == Button3)
                                             uicb_tagtransfert(s);
                                   }
                              }
                         }

                    }

                    /* SCROLL */
                    {
                         if(ev.xbutton.x < taglen[conf.ntag])
                         {
                              /* BUTTON 4 (UP) */
                              {
                                   if(ev.xbutton.button == Button4)
                                        uicb_tag("+1");
                              }
                              /* BUTTON 5 (UP) */
                              {
                                   if (ev.xbutton.button == Button5)
                                        uicb_tag("-1");
                              }
                         }

                    }


               }
          }

          /* ****** */
          /* LAYOUT */
          /* ****** */
          else if(ev.xbutton.window == layoutsym->win)
          {
               /* BUTTON 1 / 4 */
               {
                    if(ev.xbutton.button == Button1
                       || ev.xbutton.button == Button4)
                         layoutswitch(True);
               }
               /* BUTTON 3 / 5 */
               {
                    if(ev.xbutton.button == Button3
                       || ev.xbutton.button == Button5)
                         layoutswitch(False);
               }
          }

     }

     /* **** */
     /* ROOT */
     /* **** */
     {
          if(ev.xbutton.window == root)
          {
               /* BUTTON 4 */
               {
                    if(ev.xbutton.button == Button4)
                         uicb_tag("+1");
               }
               /* BUTTON 5 */
               {
                    if(ev.xbutton.button == Button5)
                         uicb_tag("-1");
               }
          }

     }

     /* *********** */
     /* BAR BUTTONS */
     /* *********** */
     {
          for(i=0; i<conf.nbutton ; ++i)
               for(j=0; j<conf.barbutton[i].nmousesec; ++j)
                    if(ev.xbutton.window == conf.barbutton[i].bw->win
                       && ev.xbutton.button == conf.barbutton[i].mouse[j])
                         if(conf.barbutton[i].func[j])
                              conf.barbutton[i].func[j](conf.barbutton[i].cmd[j]);
     }
     return;
}

/* CONFIGUREREQUEST */
void
configurerequest(XEvent ev)
{
     Client *c;
     XWindowChanges wc;
     if((c = getclient(ev.xconfigurerequest.window)))
          if(c->tile || c->lmax)
               return;
     wc.x = ev.xconfigurerequest.x;
     wc.y = ev.xconfigurerequest.y;
     wc.width = ev.xconfigurerequest.width;
     wc.height = ev.xconfigurerequest.height;
     wc.border_width = ev.xconfigurerequest.border_width;
     wc.sibling = ev.xconfigurerequest.above;
     wc.stack_mode = ev.xconfigurerequest.detail;
     XConfigureWindow(dpy, ev.xconfigurerequest.window,
                      ev.xconfigurerequest.value_mask, &wc);
     if((c = getclient(ev.xconfigurerequest.window)))
          if(wc.y < mw && wc.x < mh)
               client_moveresize(c, wc.x, wc.y, wc.width, wc.height, True);

     return;
}

/* DESTROYNOTIFY */
void
destroynotify(XEvent ev)
{
     Client *c;

     if((c = getclient(ev.xdestroywindow.window)))
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
     if((c = getclient(ev.xcrossing.window))
        || (c = client_gettbar(ev.xcrossing.window)))
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
        && (ev.xexpose.window == bar->win))
          updatebar();

     if(conf.ttbarheight)
          for(c = clients; c; c = c->next)
               if(ev.xexpose.window == c->tbar->win)
                    updatetitlebar(c);

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
     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);

     if(focused)
     {
          /* Window */
          XGrabButton(dpy, Button1, ALT, c->win, False, ButtonMask, GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
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
     for(i = 0; i < conf.nkeybind; i++)
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
     for(i = 0; i < conf.nkeybind; i++)
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
     if(!getclient(ev.xmaprequest.window))
     {
          client_focus(NULL);
          client_manage(ev.xmaprequest.window, &at);
     }

     return;
}

/* If the type is 0, this function will move, else,
 * this will resize */
void
mouseaction(Client *c, int x, int y, int type)
{
     int  ocx, ocy;
     XEvent ev;

     if(c->max || c->tile || c->lmax)
          return;

     ocx = c->x;
     ocy = c->y;
     if(XGrabPointer(dpy, root, False, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[((type) ?CurResize:CurMove)], CurrentTime) != GrabSuccess)
          return;
     if(type)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);

     for(;;)
     {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);
          if(ev.type == ButtonRelease)
          {
               if(type)
                    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
               XUngrabPointer(dpy, CurrentTime);
               updatebar();
               return;
          }
          else if(ev.type == MotionNotify)
          {
               XSync(dpy, False);
               /* Resize */
               if(type)
                    client_moveresize(c, c->x, c->y,
                                      ((ev.xmotion.x - ocx <= 0) ? 1 : ev.xmotion.x - ocx),
                                      ((ev.xmotion.y - ocy <= 0) ? 1 : ev.xmotion.y - ocy), True);
               /* Move */
               else
                    client_moveresize(c, (ocx + (ev.xmotion.x - x)),
                                      (ocy + (ev.xmotion.y - y)),
                                      c->w, c->h, True);
          }
          else if(ev.type == Expose)
               expose(ev);
     }

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
     if((c = getclient(event.xproperty.window)))
     {
          switch(event.xproperty.atom)
          {
          default: break;
          case XA_WM_TRANSIENT_FOR:
               XGetTransientForHint(dpy, c->win, &trans);
               if((c->tile || c->max) && (c->hint = (getclient(trans) != NULL)))
                    arrange();
               break;
          case XA_WM_NORMAL_HINTS:
               client_size_hints(c);
               break;
          }
          if(ev.xproperty.atom == XA_WM_NAME
             || ev.xproperty.atom == net_atom[NetWMName])
               updatetitlebar(c);
     }

     return;
}

/* UNMAPNOTIFY */
void
unmapnotify(XEvent ev)
{
     Client *c;

     if((c = getclient(ev.xunmap.window)))
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
      case UnmapNotify:       unmapnotify(event);       break;
     }

     return;
}
