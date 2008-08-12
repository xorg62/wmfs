/* Copyright (c) 1998, Regents of the University of California
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the University of California, Berkeley nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "local.h"

unsigned int numlockmask = 0;

int taglen[MAXTAG] = {3};

void
attach(Client *c) {
     if(clients)
          clients->prev = c;
     c->next = clients;
     clients = c;
     return;
}

int
clientpertag(int tag) {
     Client *c;
     int i = 0;
     for(c = clients; c; c = c->next) {
          if(c->tag == tag)
               ++i;
     }
     return i;
}

void
detach(Client *c) {
     if(c->prev) c->prev->next = c->next;
     if(c->next) c->next->prev = c->prev;
     if(c == clients) clients = c->next;
     c->next = c->prev = NULL;
     return;
}

void *
emallocz(unsigned int size) {
     void *res = calloc(1, size);
     if(!res)
          fprintf(stderr,"fatal: could not malloc() %u bytes\n", size);
     return res;
}

int
errorhandler(Display *d, XErrorEvent *event) {
     char mess[512];
     XGetErrorText(d, event->error_code, mess, 128);
     fprintf(stderr, "WMFS error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n", mess,
             event->error_code,
             event->request_code,
             event->minor_code,
             event->resourceid);
     return(1);
}

void
focus(Client *c) {
     if(sel && sel != c) {
          grabbuttons(sel, False);
          setborder(sel->win, conf.colors.bordernormal);
          setborder(sel->tbar, conf.colors.bordernormal);
     }

     if(c) grabbuttons(c, True);

     sel = c;

     if(c) {
          setborder(c->win, conf.colors.borderfocus);
          setborder(sel->tbar, conf.colors.borderfocus);
          if(conf.raisefocus)
               raiseclient(c);
          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
          updatetitle(c);
     }
     return;
}

Client*
getbutton(Window w) {
     Client *c;
     for(c = clients; c && c->button != w; c = c->next);
     return c;
}

Client*
getclient(Window w) {
     Client *c;
     for(c = clients; c && c->win != w; c = c->next);
     return c;
}

Client*
getnext(Client *c) {
     for(; c; c = c->prev);
     return c;
}

Client*
gettbar(Window w) {
     Client *c;
     for(c = clients; c && c->tbar != w; c = c->next);
     return c;
}

void
getevent(void) {
     XEvent event;
     XWindowAttributes at;
     XWindowChanges wc;
     Client *c;
     int i;
     char s[6];
     struct timeval tv;

     if(QLength(dpy) > 0) {
          XNextEvent(dpy, &event);
     } else {
          XFlush(dpy);
          FD_ZERO(&fd);
          FD_SET(ConnectionNumber(dpy), &fd);
          event.type = LASTEvent;
          tv.tv_sec = 1;
          tv.tv_usec = 0;
          if(select(FD_SETSIZE, &fd, NULL, NULL, &tv) > 0) {
               XNextEvent(dpy, &event);
          }
     }

     switch (event.type) {
     case EnterNotify:
          if(event.xcrossing.mode != NotifyNormal
             || event.xcrossing.detail == NotifyInferior) return;
          if((c = getclient(event.xcrossing.window))
             || (c = gettbar(event.xcrossing.window)))
               if(c->win != bar)
                    focus(c);
          break;

     case MapRequest:
          if(!XGetWindowAttributes(dpy, event.xmaprequest.window, &at))  return;
          if(at.override_redirect) return;
          if(!getclient(event.xmaprequest.window))
               manage(event.xmaprequest.window, &at);
          break;

     case MappingNotify:
          if(event.xmapping.request == MappingKeyboard)
               grabkeys();
          break;

     case PropertyNotify:
          if(event.xproperty.state == PropertyDelete)
               return;
          if(event.xproperty.atom == XA_WM_NAME &&
             event.xproperty.state == PropertyNewValue) {
               if((c = getclient(event.xproperty.window))) {
                    if(c->title) {
                         XFree(c->title);
                         c->title = NULL;
                         updatetitle(c);
                    }
               }
          }
          break;
     case ConfigureRequest:
          /* configure size and window position */
          wc.x = event.xconfigurerequest.x;
          wc.y = event.xconfigurerequest.y;
          wc.width = event.xconfigurerequest.width;
          wc.height = event.xconfigurerequest.height;
          wc.border_width = event.xconfigurerequest.border_width;
          wc.sibling = event.xconfigurerequest.above;
          wc.stack_mode = event.xconfigurerequest.detail;

          XConfigureWindow(dpy, event.xconfigurerequest.window,
                           event.xconfigurerequest.value_mask, &wc);
          break;

     case UnmapNotify:
          if((c = getclient(event.xunmap.window))) {
               unmanage(c);
          }
          break;

     case DestroyNotify:
          if((c = getclient(event.xdestroywindow.window))) {
               unmanage(c);
          }
          break;

     case FocusIn:
          if(sel && event.xfocus.window != sel->win)
               XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
          break;

     case KeyPress: keypress(&event); break;

     case ButtonPress:
          /* Window and Tbar */
          if((c = gettbar(event.xbutton.window))
             || (c = getclient(event.xbutton.window))) {
               raiseclient(c);
               if(event.xbutton.button == Button1)
                    mouseaction(c, event.xbutton.x_root, event.xbutton.y_root, Move);   /* type 0 for move */
               else if(event.xbutton.button == Button2)
                    tile(NULL);
               else if(event.xbutton.button == Button3)
                    mouseaction(c, event.xbutton.x_root, event.xbutton.y_root, Resize); /* type 1 for resize */
          }
          /* Button */
          else if((c = getbutton(event.xbutton.window))) {
               if(event.xbutton.button == Button1) {
                    killclient(NULL);
               } else if(event.xbutton.button == Button3)
                    togglemax(NULL);
          }
          /* Bar */
          else if(event.xbutton.window == bar) {
               for(i = 0; i < conf.ntag + 1; ++i) {
                    if(event.xbutton.x > taglen[i-1]
                       && event.xbutton.x < taglen[i]) {
                         if(event.xbutton.button == Button1) {
                              ITOA(s, i);
                              if(event.xbutton.state & ALT) {
                                   tagtransfert(s);
                                   updateall();
                              }
                              tag(s);
                              updateall();
                         }
                    }
               }
               if(sel) {
                    if(event.xbutton.x >= taglen[conf.ntag]
                       && event.xbutton.x < taglen[conf.ntag] + strlen(conf.symlayout[sel->layout])+30) {
                         if(event.xbutton.button == Button1) {
                              layoutswitch(NULL);
                         }
                    }
               }
          }
          /* Root */
          else if(event.xbutton.window == root) {
               if(event.xbutton.button == Button4)
                    tagswitch("+1");
               else if(event.xbutton.button == Button5)
                    tagswitch("-1");
          }
          break;
     }
     return;
}

void
grabbuttons(Client *c, Bool focused) {
     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     XUngrabButton(dpy, AnyButton, AnyModifier, c->tbar);
     XUngrabButton(dpy, AnyButton, AnyModifier, c->button);

     if(focused) {
          /* Window */
          XGrabButton(dpy, Button1, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, ALT|LockMask, c->win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT|LockMask, c->win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT|LockMask, c->win, False, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          /* Titlebar */
          XGrabButton(dpy, Button1, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          /* Button */
          XGrabButton(dpy, Button1, AnyModifier, c->button, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, AnyModifier, c->button, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
     } else {
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, AnyButton, AnyModifier, c->tbar, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);


          XGrabButton(dpy, AnyButton, AnyModifier, c->button, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
     }
}

void
grabkeys(void) {
     unsigned int i;
     KeyCode code;
     XUngrabKey(dpy, AnyKey, AnyModifier, root);
     for(i = 0; i < conf.nkeybind; i++) {
          code = XKeysymToKeycode(dpy, keys[i].keysym);
          XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|numlockmask, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask, root, True, GrabModeAsync, GrabModeAsync);
          XGrabKey(dpy, code, keys[i].mod|LockMask|numlockmask, root, True, GrabModeAsync, GrabModeAsync);
     }
     return;
}

void
hide(Client *c) {
     if(c) {
          XMoveWindow(dpy, c->win, c->x, c->y+mh*2);
          XMoveWindow(dpy, c->tbar, c->x, c->y+mh*2);
          XMoveWindow(dpy, c->button, c->x, c->y+mh*2);
     }
}

void
init(void) {
     XSetWindowAttributes at;
     XModifierKeymap *modmap;
     int i, j;

     /* FIRST INIT */
     gc = DefaultGC (dpy, screen);
     screen = DefaultScreen (dpy);
     root = RootWindow (dpy, screen);
     mw = DisplayWidth (dpy, screen);
     mh = DisplayHeight (dpy, screen);
     seltag = 1;
     init_conf();

     /* INIT FONT */
     font = XLoadQueryFont(dpy, conf.font);
     if(!font){
          fprintf(stderr, "XLoadQueryFont: failed loading font '%s'\n", conf.font);
          exit(0);

     }
     XSetFont(dpy, gc, font->fid);
     fonth = font->ascent + font->descent;
     barheight = fonth + 3;

     /* INIT CURSOR */
     cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
     cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
     cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

     /* INIT MODIFIER */
     modmap = XGetModifierMapping(dpy);
     for(i = 0; i < 8; i++)
          for(j = 0; j < modmap->max_keypermod; j++) {
               if(modmap->modifiermap[i * modmap->max_keypermod + j]
                  == XKeysymToKeycode(dpy, XK_Num_Lock))
                    numlockmask = (1 << i);
          }
     XFreeModifiermap(modmap);

     /* INIT ATOM */
     wm_atom[WMState] = XInternAtom(dpy, "WM_STATE", False);
     wm_atom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
     wm_atom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
     net_atom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
     net_atom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
     XChangeProperty(dpy, root, net_atom[NetSupported], XA_ATOM, 32,
                     PropModeReplace, (unsigned char *) net_atom, NetLast);

     /* INIT ROOT */
     at.event_mask = KeyMask | ButtonPressMask | ButtonReleaseMask |
          SubstructureRedirectMask | SubstructureNotifyMask |
          EnterWindowMask | LeaveWindowMask | StructureNotifyMask ;
     at.cursor = cursor[CurNormal];
     XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &at);

     /* INIT BAR */
     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     at.event_mask = ButtonPressMask | ExposureMask;
     bar = XCreateWindow(dpy, root, 0, 0, mw, barheight, 0, DefaultDepth(dpy, screen),
                         CopyFromParent, DefaultVisual(dpy, screen),
                         CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);
     XSetWindowBackground(dpy, bar, conf.colors.bar);
     XMapRaised(dpy, bar);

     /* INIT STUFF */
     XSetErrorHandler(errorhandler);
     grabkeys();

    return;
}

Bool
ishide(Client *c) {
     int i;
     for(i = 0; i < conf.ntag+1; ++i)
          if(c->tag == i && seltag == i)
               return False;
     return True;
}

void
keymovex(char *cmd) {
     if(sel && cmd && !ishide(sel) && !sel->max) {
          int tmp;
          tmp = sel->x + atoi(cmd);
          moveresize(sel,tmp, sel->y, sel->w, sel->h);
     }
     return;
}

void
keymovey(char *cmd) {
     if(sel && cmd && !ishide(sel) && !sel->max) {
          int tmp;
          tmp = sel->y + atoi(cmd);
          moveresize(sel, sel->x, tmp, sel->w, sel->h);
     }
     return;
}

void
keypress(XEvent *e) {
     unsigned int i;
     KeySym keysym;
     XKeyEvent *ev;
     ev = &e->xkey;
     keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
     for(i = 0; i < conf.nkeybind; i++)
          if(keysym == keys[i].keysym
             && (keys[i].mod & ~(numlockmask | LockMask)) ==
             (ev->state & ~(numlockmask | LockMask))
             && keys[i].func)
          {
               keys[i].func(keys[i].cmd);
               updateall();
          }
     return;
}

void
keyresize(char *cmd) {
     if(sel && !ishide(sel) && !sel->max) {
          int temph=0, tempw=0, modh=0, modw=0,
               tmp=0;

          switch(cmd[1]) {
               case 'h': tmp = (cmd[0] == '+') ? 5 : -5; modh = tmp; break;
               case 'w': tmp = (cmd[0] == '+') ? 5 : -5; modw = tmp; break;
          }

          temph = sel->h + modh;
          tempw = sel->w + modw;
          temph = (temph < 10) ? 10 : temph;
          tempw = (tempw < 10) ? 10 : tempw;
          moveresize(sel, sel->x, sel->y, tempw, temph);
     }
     return;
}

void
killclient(char *cmd) {
     if(sel && !ishide(sel)) {
          XEvent ev;
          ev.type = ClientMessage;
          ev.xclient.window = sel->win;
          ev.xclient.message_type = wm_atom[WMProtocols];
          ev.xclient.format = 32;
          ev.xclient.data.l[0] = wm_atom[WMDelete];
          ev.xclient.data.l[1] = CurrentTime;
          XSendEvent(dpy, sel->win, False, NoEventMask, &ev);
     }
     return;
}

void
layoutswitch(char *cmd) {
     if(sel) {
          switch(sel->layout) {
          case Free: tile(NULL);      break;
          case Tile: togglemax(NULL); break;
          case Max:  togglemax(NULL); break;
          }
     }
     return;
}

void
mapclient(Client *c) {
     if(c) {
          XMapWindow(dpy, c->win);
          XMapWindow(dpy, c->tbar);
          XMapWindow(dpy, c->button);
     }
     return;
}

void
manage(Window w, XWindowAttributes *wa) {
     Client *c, *t = NULL;
     Window trans;
     Status rettrans;
     XWindowChanges winc;

     c = emallocz(sizeof(Client));
     c->win = w;
     c->x = wa->x;
     c->y = wa->y + conf.ttbarheight + barheight;
     c->w = wa->width;
     c->h = wa->height;
     c->border = wa->border_width;
     c->tag = seltag;
     c->layout = Free;

     setborder(w, conf.colors.bordernormal);

     XConfigureWindow(dpy, w, CWBorderWidth, &winc);

     XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask |
                  PropertyChangeMask | StructureNotifyMask);

     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);

     c->tbar = XCreateSimpleWindow(dpy,root,
                                   c->x,
                                   c->y - conf.ttbarheight,
                                   c->w,
                                   conf.ttbarheight,
                                   conf.borderheight,
                                   conf.colors.bordernormal,
                                   conf.colors.bar);
     XSelectInput(dpy, c->tbar, ExposureMask | EnterWindowMask);

     c->button = XCreateSimpleWindow(dpy,root,
                                     c->x + c->w - 10,
                                     BUTY(c->y),
                                     5,
                                     BUTH,
                                     1,
                                     conf.colors.button,
                                     conf.colors.button);
     XSelectInput(dpy, c->button, ExposureMask | EnterWindowMask);

     grabbuttons(c, False);
     setsizehints(c);
     attach(c);
     moveresize(c, c->x, c->y, c->w, c->h);
     XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
     mapclient(c);
     updatetitle(c);
     setborder(c->tbar, conf.colors.bordernormal);
     focus(c);
     return;
}

/* If the type is 0, this function will move, else,
   this will resize */

void
mouseaction(Client *c, int x, int y, int type) {
     int  ocx, ocy;
     XEvent ev;

     if(c->max)
          return;

     ocx = c->x;
     ocy = c->y;

     if(XGrabPointer(dpy, root, 0, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[((type) ?CurResize:CurMove)], CurrentTime) != GrabSuccess) return;
     if(type)
          XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);

     for(;;) {
          XMaskEvent(dpy, MouseMask | ExposureMask | SubstructureRedirectMask, &ev);
          if(ev.type == ButtonRelease) {
               if(type)
                    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w, c->h);
               XUngrabPointer(dpy, CurrentTime);
               return;
          } else if(ev.type == MotionNotify) {

               XSync(dpy, 0);

               if(type)  /* Resize */
                    moveresize(c, c->x, c->y,
                               ((ev.xmotion.x - ocx <= 0) ? 1 : ev.xmotion.x - ocx),
                               ((ev.xmotion.y - ocy <= 0) ? 1 : ev.xmotion.y - ocy));
               else     /* Move */
                    moveresize(c,
                               (ocx + (ev.xmotion.x - x)),
                               (ocy + (ev.xmotion.y - y)),
                               c->w, c->h);

               if(conf.clientbarblock) {
                    if(c->y  < barheight + conf.ttbarheight - 5) {
                         moveresize(c, c->x, barheight+conf.ttbarheight, c->w, c->h);
                         XUngrabPointer(dpy, CurrentTime);
                         return;
                    }
               }
          }
     }
     return;
}

void
moveresize(Client *c, int x, int y, int w, int h) {
     if(c) {
          /* Resize hints {{{ */

          if(c->minw > 0 && w < c->minw) w = c->minw;
          if(c->minh > 0 && h < c->minh) h = c->minh;
          if(c->maxw > 0 && w > c->maxw) w = c->maxw;
          if(c->maxh > 0 && h > c->maxh) h = c->maxh;
          if(w <= 0 || h <= 0) return;

          /* }}} */

          c->layout = Free;
          c->max = False;
          if(c->x != x || c->y != y || c->w != w || c->h != h) {
               c->x = x;
               c->y = y;
               c->w = w;
               c->h = h;

               if(conf.clientbarblock) {
                    if((y-conf.ttbarheight) <= barheight)
                         y = barheight+conf.ttbarheight;
               } else updatebar();

          XMoveResizeWindow(dpy, c->win, x, y, w ,h);
          XMoveResizeWindow(dpy, c->tbar, x, y - conf.ttbarheight, w, conf.ttbarheight);
          XMoveResizeWindow(dpy, c->button,
                            (x + w - 10),
                            BUTY(y),
                            5,
                            BUTH);
          updateall();
          XSync(dpy, False);
          }
     }
     return;
}

void
raiseclient(Client *c) {
     if(c) {
          XRaiseWindow(dpy,c->win);
          XRaiseWindow(dpy,c->tbar);
          XRaiseWindow(dpy,c->button);
     }
     return;
}

void
scan(void) {
     unsigned int i, num;
     Window *wins, d1, d2;
     XWindowAttributes wa;

     wins = NULL;
     if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
          for(i = 0; i < num; i++) {
               if(!XGetWindowAttributes(dpy, wins[i], &wa))
                    continue;
               if(wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                    continue;
               if(wa.map_state == IsViewable)
                    manage(wins[i], &wa);
          }
     }
     if(wins)
          XFree(wins);
     return;
}

void
setborder(Window win, int color) {
     XSetWindowBorder(dpy, win, color);
     XSetWindowBorderWidth(dpy, win, conf.borderheight);
     return;
}

void
setsizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
             size.flags = PSize;
	if(size.flags & PBaseSize) {
             c->basew = size.base_width; c->baseh = size.base_height;
        }
	else if(size.flags & PMinSize) {
             c->basew = size.min_width; c->baseh = size.min_height;
	}
	else c->basew = c->baseh = 0;
	if(size.flags & PResizeInc) {
             c->incw = size.width_inc; c->inch = size.height_inc;
	}
	else c->incw = c->inch = 0;
	if(size.flags & PMaxSize) {
             c->maxw = size.max_width; c->maxh = size.max_height;
	}
	else c->maxw = c->maxh = 0;
	if(size.flags & PMinSize) {
             c->minw = size.min_width; c->minh = size.min_height;
	}
	else if(size.flags & PBaseSize) {
             c->minw = size.base_width; c->minh = size.base_height;
	}
	else c->minw = c->minh = 0;
}

void
spawn(char *cmd) {
     if(strlen(cmd) > 0 && !fork()) {
          execl(getenv("SHELL"), "sh", "-c", cmd, NULL);
          exit(1);
     }
     return;
}

void
tag(char *cmd) {
     Client *c;
     int tmp = atoi(cmd);

     if(tmp > conf.ntag || tmp < 1 || tmp == seltag)
          return;
     for(c = clients; c; c = c->next) {
          if(!ishide(c))
               hide(c);
          if(c->tag == tmp)
               unhide(c);
     }

     seltag = tmp;
     sel = NULL;
     return;
}

void
tagswitch(char *cmd) {
     Client *c;
     int tmp;

     tmp = atoi(cmd);

     if(seltag + tmp > conf.ntag || seltag + tmp < 1)
          return;

     seltag += tmp;
     for(c = clients; c; c = c->next) {
          if(c->tag == seltag - tmp)
               hide(c);
          if(c->tag == seltag)
               unhide(c);
     }
     sel = NULL;
     return;
}

void
tagtransfert(char *cmd) {
     int n = atoi(cmd);
     if(!sel)
          return;
     sel->tag = n;
     if(n != seltag)
          hide(sel);
     if(n == seltag)
          unhide(sel);
}

void
tile(char *cmd) {
     if(sel) {
          Client *c;
          int i;
          unsigned int x, y, w, h, bord;
          unsigned int barto;

          barto = conf.ttbarheight + barheight;
          bord =  conf.borderheight * 2;

          x = mw / 2 + conf.borderheight;
          y = barto;
          w = ((mw - bord ) / 2 - bord);
          if(clientpertag(seltag)-1)
               h = ((mh-bord) - conf.ttbarheight - barheight) / (clientpertag(seltag) - 1) ;

          sel->ox = sel->x;
          sel->oy = sel->y;
          sel->ow = sel->w;
          sel->oh = sel->h;

          /* Master client in first (always the sel window) */
          moveresize(sel, 0, barto,
                     ((clientpertag(seltag) > 1) ? (mw-bord) / 2 : (mw-bord)),
                     ((mh-bord) - conf.ttbarheight - barheight));
          sel->layout = Tile;

          /* tiling */
          for(i=0, c = clients; c; c = c->next, ++i) {
               if(c != sel && !ishide(c)) {
                    c->ox = c->x;
                    c->oy = c->y;
                    c->ow = c->w;
                    c->oh = c->h;
                    moveresize(c, x, y, w, h);
                    if(i < i + 1)
                         y = c->y + c->h + bord + conf.ttbarheight;
                   c->layout = Tile;
               }
          }
     }
     return;
}
void
togglemax(char *cmd) {
     if(sel && !ishide(sel)) {
          if(!sel->max) {
               sel->ox = sel->x;
               sel->oy = sel->y;
               sel->ow = sel->w;
               sel->oh = sel->h;
               moveresize(sel, 0,
                          conf.ttbarheight + barheight,
                          (mw-(conf.borderheight * 2)),
                          (mh-(conf.borderheight * 2)- conf.ttbarheight - barheight));
               sel->max = True;
               sel->layout = Max;
          } else if(sel->max) {
               raiseclient(sel);
               moveresize(sel, sel->ox, sel->oy, sel->ow, sel->oh);
               sel->max = False;
               sel->layout = Free;
          }
     }
     return;
}

void
unhide(Client *c) {
     if(c) {
          XMoveWindow(dpy,c->win,c->x,c->y);
          XMoveWindow(dpy,c->tbar,c->x,
                      (c->y - conf.ttbarheight));
          XMoveWindow(dpy,c->button,
                      (c->x + c->w -10),
                      (c->y - 9));
          if(conf.clientbarblock)
               if(c->y+conf.ttbarheight <= barheight)
                    moveresize(c, c->x, barheight + conf.ttbarheight, c->w, c->h);
     }
}

void
unmanage(Client *c) {
     XSetErrorHandler(errorhandler);
     if(sel == c)
          sel = c->next;
     else
          sel = NULL;
     XUnmapWindow(dpy, c->tbar);
     XDestroyWindow(dpy, c->tbar);
     XUnmapWindow(dpy, c->button);
     XDestroyWindow(dpy, c->button);
     detach(c);
     free(c);
     XSync(dpy, False);
     return;
}

void
updateall(void) {
     Client *c;
     for(c = clients; c; c = c->next) {
          if(!ishide(c))
             updatetitle(c);
     }
}

void
updatebar(void) {
     int  i,j;
     char buf[conf.ntag][100];
     tm = localtime(&lt);
     lt = time(NULL);
     if(!conf.clientbarblock)
          XRaiseWindow(dpy, bar);
     XClearWindow(dpy, bar);
     XSetForeground(dpy, gc, conf.colors.text);

     for(i=0;i< conf.ntag;++i) {
          /* Make the tag string */
          if(clientpertag(i+1))
               sprintf(buf[i], "%s(%d) ", conf.taglist[i], clientpertag(i+1));
          else
               sprintf(buf[i], "%s() ", conf.taglist[i]);
          taglen[i+1] = taglen[i] + 6*(strlen(conf.taglist[i]) + ((clientpertag(i+1) >= 10) ? 5 : 4));
          /* Rectangle for the tag background */
          if(i+1 == seltag) XSetForeground(dpy, gc, conf.colors.tagselbg);
          else XSetForeground(dpy, gc, 0x090909);
          XFillRectangle(dpy, bar, gc, taglen[i]-4, 0, strlen(buf[i])*6, barheight);
          /* Draw tag */
          if(i+1 == seltag) XSetForeground(dpy, gc, conf.colors.tagselfg);
          else XSetForeground(dpy, gc, conf.colors.text);
          XDrawString(dpy, bar, gc, taglen[i], fonth-1, buf[i], strlen(buf[i]));
     }

     /* Draw layout symbol */
     XSetForeground(dpy, gc, conf.colors.tagselfg);
     XDrawString(dpy, bar, gc, taglen[conf.ntag],
                 fonth-1,
                 (sel) ? conf.symlayout[sel->layout] : conf.symlayout[Free],
                 (sel) ? strlen(conf.symlayout[sel->layout]) :strlen(conf.symlayout[Free]) );

     /* Draw date */
     sprintf(status, "%02d:%02d WMFS", tm->tm_hour, tm->tm_min);
     j = strlen(status);
     XSetForeground(dpy, gc, conf.colors.text);
     XDrawString(dpy, bar, gc, mw- j*6, fonth -1 , status, j);
     XDrawLine(dpy, bar, gc, mw-j*6-5, 0 , mw-j*6-5, barheight);
     XSync(dpy, False);
     return;
}

void
unmapclient(Client *c) {
     if(c) {
          XUnmapWindow(dpy, c->win);
          XUnmapWindow(dpy, c->tbar);
          XUnmapWindow(dpy, c->button);
     }
     return;
}

void
updatetitle(Client *c) {
     XFetchName(dpy, c->win, &(c->title));
     if(!c->title)
          c->title = strdup("WMFS");
     XClearWindow(dpy, c->tbar);
     XSetForeground(dpy, gc, conf.colors.text);
     XDrawString(dpy, c->tbar, gc, 5, 10, c->title, strlen(c->title));
     return;
}

void
wswitch(char *cmd) {
     if(sel && !ishide(sel)) {
          Client *c;
          if(cmd[0] == '+') {
               for(c = sel->next; c && ishide(c); c = c->next);
               if(!c)
                    for(c = clients; c && ishide(c); c = c->next);
               if(c) {
                    focus(c);
                    raiseclient(c);
               }
          } else if(cmd[0] == '-') {
               for(c = sel->prev; c && ishide(c); c = c->prev);
               if(!c) {
                    for(c = clients; c && c->next; c = c->next);
                    for(; c && ishide(c); c = c->prev);
               }
               if(c) {
                    focus(c);
                    raiseclient(c);
               }
          }
     }
     return;
}

int
main(int argc,char **argv) {
     dpy = XOpenDisplay(NULL);
     int i;

     static struct option long_options[] ={
          {"help",	 	0, NULL, 'h'},
          {"info",		0, NULL, 'i'},
          {"version",     	0, NULL, 'v'},
          {NULL,		0, NULL, 0}
     };

     while ((i = getopt_long (argc, argv, "hvi", long_options, NULL)) != -1) {
          switch (i) {
          case 'h':
          default:
               printf("Usage: wmfs [OPTION]\n"
                      "   -h, --help         show this page\n"
                      "   -i, --info         show informations\n"
                      "   -v, --version      show WMFS version\n");
               exit(EXIT_SUCCESS);
               break;
          case 'i':
               printf("WMFS - Window Manager From Scratch. By :\n"
                      "   - Martun Duquesnoy (code)\n"
                      "   - Marc Lagrange (build system)\n");
               exit(EXIT_SUCCESS);
               break;
          case 'v':
               printf("WMFS version : "WMFS_VERSION".\n"
                      "  Compilation settings :\n"
                      "    - Flags : "WMFS_COMPILE_FLAGS"\n"
                      "    - Linked Libs : "WMFS_LINKED_LIBS"\n"
                      "    - On "WMFS_COMPILE_MACHINE" by "WMFS_COMPILE_BY".\n");
               exit(EXIT_SUCCESS);
               break;
          }
     }

     if(!dpy) {printf("wmfs: cannot open X server\n"); exit(1);}

     init();
     scan();

     for(;;) {
          getevent();
          updatebar();
     }

     XCloseDisplay(dpy);
     return 0;
}

