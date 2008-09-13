#include "wmfs.h"

/* BUTTONPRESS */
void
buttonpress(XEvent ev) {
     Client *c;
     int i, j;
     char s[6];

     /* Tbar'n'Button */
     if(conf.ttbarheight) {
          if((c = gettbar(ev.xbutton.window))) {
               raiseclient(c);
               if(ev.xbutton.button == Button1)
                    mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Move);   /* type 0 for move */
               else if(ev.xbutton.button == Button2)
                    tile_switch(NULL);
               else if(ev.xbutton.button == Button3)
                    mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Resize); /* type 1 for resize */
          } else if((c = getbutton(ev.xbutton.window))) {
               if(ev.xbutton.button == Button1)
                    killclient(NULL);
               else if(ev.xbutton.button == Button3)
                    togglemax(NULL);
          }
     }
     /* Window */
     if((c = getclient(ev.xbutton.window))) {
          raiseclient(c);
          if(ev.xbutton.button == Button1)
               mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Move);   /* type 0 for move */
          else if(ev.xbutton.button == Button2)
               togglemax(NULL);
          else if(ev.xbutton.button == Button3)
               mouseaction(c, ev.xbutton.x_root, ev.xbutton.y_root, Resize); /* type 1 for resize */
     }
     /* Bar */
     /* for tag click */
     else if(ev.xbutton.window == bar) {
          for(i = 0; i < conf.ntag + 1; ++i) {
               if(ev.xbutton.x > taglen[i-1]
                  && ev.xbutton.x < taglen[i]) {
                    ITOA(s, i);
                    if(ev.xbutton.button == Button1)
                         tag(s);
                    else if(ev.xbutton.button == Button3)
                         tagtransfert(s);
               }
          }
          /* tag switch with scroll */
          if(ev.xbutton.x < taglen[conf.ntag]) {
               if(ev.xbutton.button == Button4)
                    tag("+1");
               else if(ev.xbutton.button == Button5)
                    tag("-1");
          }
          /* layout switch */
          if(ev.xbutton.x >= taglen[conf.ntag]
             && ev.xbutton.x < taglen[conf.ntag] +
             (strlen((getlayoutsym(layout[seltag])))*fonty+3)) {
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
     else if(ev.xbutton.window == root) {
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
configurerequest(XEvent ev) {
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
     if((c = getclient(ev.xconfigurerequest.window))) {
          if(wc.y < mw && wc.x < mh) {
               c->free = True;
               c->max = False;
               c->tile = False;
               moveresize(c, wc.x, wc.y, wc.width, wc.height, 1);
               arrange();
          }
     }
     return;
}

/* DESTROYNOTIFY */
void
destroynotify(XEvent ev) {
     Client *c;
     if((c = getclient(ev.xdestroywindow.window)))
          unmanage(c);
}

/* ENTERNOTIFY */
void
enternotify(XEvent ev) {
     Client *c;

     if(ev.xcrossing.mode != NotifyNormal
        || ev.xcrossing.detail == NotifyInferior)
          return;
     if((c = getclient(ev.xcrossing.window))
        || (c = gettbar(ev.xcrossing.window)))
          if(c->win != bar)
               focus(c);
     return;
}

/* FOCUSIN */
void
focusin(XEvent ev) {
     if(sel && ev.xfocus.window != sel->win)
          XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
     return;
}

/* KEYPRESS */
void
keypress(XEvent ev) {
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
mapnotify(XEvent ev) {
     if(ev.xmapping.request == MappingKeyboard)
          grabkeys();
     return;
}


/* MAPREQUEST */
void
maprequest(XEvent ev) {
     XWindowAttributes at;

     if(!XGetWindowAttributes(dpy, ev.xmaprequest.window, &at))  return;
     if(at.override_redirect) return;
     if(!getclient(ev.xmaprequest.window))
          manage(ev.xmaprequest.window, &at);
     return;
}

/* PROPERTYNOTIFY */
void
propertynotify(XEvent ev) {
     Client *c;
     Window trans;

     if(event.xproperty.state == PropertyDelete)
          return;
     if((c = getclient(event.xproperty.window))) {
          switch(event.xproperty.atom) {
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
               updateall();
     }
     return;
}

/* UNMAPNOTIFY */
void
unmapnotify(XEvent ev) {
     Client *c;

     if((c = getclient(ev.xunmap.window)))
          if(!c->hide)
               unmanage(c);
     return;
}

/* Handle */
void
getevent(void) {
     struct timeval tv;

     if(QLength(dpy) > 0)
          XNextEvent(dpy, &event);
     else {
          XFlush(dpy);
          FD_ZERO(&fd);
          FD_SET(ConnectionNumber(dpy), &fd);
          event.type = LASTEvent;
          tv.tv_sec = 60;
          tv.tv_usec = 0;
          if(select(FD_SETSIZE, &fd, NULL, NULL, &tv) > 0)
               XNextEvent(dpy, &event);
     }

     switch (event.type)
     {
     case ButtonPress:       buttonpress(event);       break;
     case ConfigureRequest:  configurerequest(event);  break;
     case DestroyNotify:     destroynotify(event);     break;
     case EnterNotify:       enternotify(event);       break;
     case FocusIn:           focusin(event);           break;
     case KeyPress:          keypress(event);          break;
     case MapRequest:        maprequest(event);        break;
     case MappingNotify:     mapnotify(event);         break;
     case PropertyNotify:    propertynotify(event);    break;
     case UnmapNotify:       unmapnotify(event);       break;
     }

     return;
}
