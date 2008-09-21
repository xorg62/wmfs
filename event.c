/*
*      event.c
*      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.con>
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
     int i, j;
     char s[6];

     /* Tbar'n'Button */
     if(conf.ttbarheight)
     {
          if((c = gettbar(ev.xbutton.window)))
          {
               raiseclient(c);
               if(ev.xbutton.button == Button1)
                    mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Move);
               else if(ev.xbutton.button == Button2)
                    tile_switch(NULL);
               else if(ev.xbutton.button == Button3)
                    mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Resize);
          }
          else if((c = getbutton(ev.xbutton.window)))
          {
               if(ev.xbutton.button == Button1)
                    killclient(NULL);
               else if(ev.xbutton.button == Button3)
                    togglemax(NULL);
          }
     }
     /* Window */
     if((c = getclient(ev.xbutton.window)))
     {
          raiseclient(c);
          if(ev.xbutton.button == Button1)
               mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Move);
          else if(ev.xbutton.button == Button2)
               togglemax(NULL);
          else if(ev.xbutton.button == Button3)
               mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Resize);
     }
     /* Bar */
     /* for tag click */
     else if(ev.xbutton.window == bar)
     {
          for(i = 0; i < conf.ntag + 1; ++i)
          {
               if(ev.xbutton.x > taglen[i-1] - 3
                  && ev.xbutton.x < taglen[i])
               {
                    ITOA(s, i);
                    if(ev.xbutton.button == Button1)
                         tag(s);
                    else if(ev.xbutton.button == Button3)
                         tagtransfert(s);
               }
          }
          /* tag switch with scroll */
          if(ev.xbutton.x < taglen[conf.ntag])
          {
               if(ev.xbutton.button == Button4)
                    tag("+1");
               else if(ev.xbutton.button == Button5)
                    tag("-1");
          }
          /* layout switch */
          if(ev.xbutton.x >= taglen[conf.ntag] - 3
             && ev.xbutton.x < taglen[conf.ntag] +
             (strlen((getlayoutsym(seltag)))*fonty+3) - 3)
          {
               if(ev.xbutton.button == Button1
                  || ev.xbutton.button == Button4)
                    layoutswitch("+");
               else if(ev.xbutton.button == Button3
                       || ev.xbutton.button == Button5)
                    layoutswitch("-");
          }
     }
     /* Root */
     /* tag switch */
     else if(ev.xbutton.window == root)
     {
          if(ev.xbutton.button == Button4)
               tag("+1");
          else if(ev.xbutton.button == Button5)
               tag("-1");
     }
     /* Bar Button */
     for(i=0; i<conf.nbutton ; ++i)
          for(j=0; j<conf.barbutton[i].nmousesec; ++j)
               if(ev.xbutton.window == conf.barbutton[i].win
                  && ev.xbutton.button == conf.barbutton[i].mouse[j])
                    if(conf.barbutton[i].func[j])
                         conf.barbutton[i].func[j](conf.barbutton[i].cmd[j]);
     return;
}

/* CONFIGUREREQUEST */
void
configurerequest(XEvent ev)
{
     Client *c;
     XWindowChanges wc;
     if((c = getclient(ev.xconfigurerequest.window)))
          if(c->tile)
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
               moveresize(c, wc.x, wc.y, wc.width, wc.height, 1);
     return;
}

/* DESTROYNOTIFY */
void
destroynotify(XEvent ev)
{
     Client *c;

     if((c = getclient(ev.xdestroywindow.window)))
          unmanage(c);
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
        || (c = gettbar(ev.xcrossing.window)))
          focus(c);
     else
          focus(NULL);
     return;
}

/* EXPOSE */
void
expose(XEvent ev)
{
     Client *c;

     if(ev.xexpose.count == 0
        && (ev.xexpose.window == bar))
          updatebar();
     for(c = clients; c; c = c->next)
          if(conf.ttbarheight > 10
             && ev.xexpose.window == c->tbar)
               updatetitle(c);

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

/* KEYPRESS */
void
keypress(XEvent ev)
{
     unsigned int i;
     KeySym keysym;

     keysym = XKeycodeToKeysym(dpy, (KeyCode)ev.xkey.keycode, 0);
     for(i = 0; i < conf.nkeybind; i++)
          if(keysym == keys[i].keysym
             && (keys[i].mod & ~(numlockmask | LockMask)) ==
             (ev.xkey.state & ~(numlockmask | LockMask))
             && keys[i].func)
          {
               keys[i].func(keys[i].cmd);
               updateall();
          }
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
          focus(NULL);
          manage(ev.xmaprequest.window, &at);
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
               setsizehints(c);
               break;
          }
          if(ev.xproperty.atom == XA_WM_NAME
             || ev.xproperty.atom == net_atom[NetWMName])
               updatetitle(c);
     }
     return;
}

/* UNMAPNOTIFY */
void
unmapnotify(XEvent ev)
{
     Client *c;

     if((c = getclient(ev.xunmap.window)))
         unmanage(c);
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
