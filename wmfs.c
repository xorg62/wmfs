/*
*      wmfs.c
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

void
arrange(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c))
               unhide(c);
          else
               hide(c);

     if(sel)
          tags[seltag].layout.func();
     focus(selbytag[seltag]);
     updatebar();

     return;
 }

void
attach(Client *c)
{
     if(clients)
          clients->prev = c;
     c->next = clients;
     clients = c;

     return;
}


int
clientpertag(int tag)
{
     Client *c;
     int i = 0;

     for(c = clients; c; c = c->next)
          if(c->tag == tag)
               ++i;
     return i;
}

/* True : next
 * False : prev */
void
client_switch(Bool b)
{
     Client *c;

     if(!sel || ishide(sel))
          return;
     if(b)
     {
          for(c = sel->next; c && ishide(c); c = c->next);
          if(!c)
               for(c = clients; c && ishide(c); c = c->next);
          if(c)
          {
               focus(c);
               if(!c->tile)
                    raiseclient(c);
          }
     }
     else
     {
          for(c = sel->prev; c && ishide(c); c = c->prev);
          if(!c)
          {
               for(c = clients; c && c->next; c = c->next);
               for(; c && ishide(c); c = c->prev);
          }
          if(c)
          {
               focus(c);
               if(!c->tile)
                    raiseclient(c);
          }
     }

     return;
}

void
uicb_client_prev(uicb_t cmd)
{
     client_switch(False);

     return;
}

void
uicb_client_next(uicb_t cmd)
{
     client_switch(True);

     return;
}


void
detach(Client *c)
{
     Client **cc;

     for(cc = &clients; *cc && *cc != c; cc = &(*cc)->next);
     *cc = c->next;

     return;
}

int
errorhandler(Display *d, XErrorEvent *event)
{
     char mess[512];

     XGetErrorText(d, event->error_code, mess, 128);
     fprintf(stderr, "WMFS error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n", mess,
             event->error_code,
             event->request_code,
             event->minor_code,
             event->resourceid);

     return 1;
}

/* for no-important error */
int
errorhandlerdummy(Display *d, XErrorEvent *event)
{
     return 0;
}

void
focus(Client *c)
{
     Client *cc;

     if(sel && sel != c)
     {
          grabbuttons(sel, False);
          setborder(sel->win, conf.colors.bordernormal);
          if(conf.ttbarheight)
               setborder(sel->tbar->win, conf.colors.bordernormal);

     }
     if(c)
          grabbuttons(c, True);

     sel = c;
     selbytag[seltag] = sel;

     if(c)
     {
          setborder(c->win, conf.colors.borderfocus);
          if(conf.ttbarheight)
               setborder(sel->tbar->win, conf.colors.borderfocus);
          if(conf.raisefocus)
               raiseclient(c);
          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
          updatetitlebar(c);
     }
     else
          XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);

     for(cc = clients; cc; cc = cc->next)
          if(!ishide(cc))
               updatetitlebar(cc);

     return;
}

Client*
getbutton(Window w)
{
     Client *c;

     for(c = clients; c && c->button != w; c = c->next);

     return c;
}

Client*
getclient(Window w)
{
     Client *c;

     for(c = clients; c && c->win != w; c = c->next);

     return c;
}

Client*
getnext(Client *c)
{
     for(; c; c = c->prev);

     return c;
}

Client*
gettbar(Window w)
{
     Client *c;

     if(!conf.ttbarheight)
          return NULL;

     for(c = clients; c && c->tbar->win != w; c = c->next);

     return c;
}

void
grabbuttons(Client *c, Bool focused)
{
     int i;

     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     if(conf.ttbarheight)
     {
          XUngrabButton(dpy, AnyButton, AnyModifier, c->tbar->win);
          if(conf.ttbarheight > 5)
               XUngrabButton(dpy, AnyButton, AnyModifier, c->button);
     }

     if(focused)
     {
          /* Window */
          XGrabButton(dpy, Button1, ALT, c->win, False, ButtonMask, GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT|LockMask, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          if(conf.ttbarheight)
          {
               /* Titlebar */
               XGrabButton(dpy, Button1, AnyModifier, c->tbar->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, Button2, AnyModifier, c->tbar->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, Button3, AnyModifier, c->tbar->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               /* Titlebar Button */
               if(conf.ttbarheight > 5)
               {
                    XGrabButton(dpy, Button1, AnyModifier, c->button, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
                    XGrabButton(dpy, Button3, AnyModifier, c->button, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               }
          }
          /* Bar Button */
          for(i=0; i< conf.nbutton; ++i)
               XGrabButton(dpy, Button1, AnyModifier, conf.barbutton[i].bw->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
     }
     else
     {
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          if(conf.ttbarheight)
          {
               XGrabButton(dpy, AnyButton, AnyModifier, c->tbar->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               if(conf.ttbarheight > 5)
                    XGrabButton(dpy, AnyButton, AnyModifier, c->button, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          }
          for(i=0; i< conf.nbutton; ++i)
               XGrabButton(dpy, Button1, AnyModifier, conf.barbutton[i].bw->win, False, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
     }

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

void
hide(Client *c)
{
     if(!c)
          return;
     XMoveWindow(dpy, c->win, c->x, c->y+mh*2);
     if(conf.ttbarheight)
     {
          bar_moveresize(c->tbar, c->x, c->y+mh*2, c->w, c->h);
          if(conf.ttbarheight > 5)
               XMoveWindow(dpy, c->button, c->x, c->y+mh*2);
     }
     //unmapclient(c);
     setwinstate(c->win, IconicState);
     c->hide = True;

     return;
}

void
init(void)
{
     XSetWindowAttributes at;
     XModifierKeymap *modmap;
     int i, j;

     /* FIRST INIT */
     gc = DefaultGC (dpy, screen);
     screen = DefaultScreen (dpy);
     root = RootWindow (dpy, screen);
     mw = DisplayWidth (dpy, screen);
     mh = DisplayHeight (dpy, screen);

     /* INIT TAG / LAYOUT ATTRIBUTE */
     seltag = 1;
     for(i = 0; i < conf.ntag + 1; ++i)
          tags[i] = conf.tag[i - 1];

     /* INIT FONT */
     xftfont = XftFontOpenName(dpy, screen, conf.font);
     if(!xftfont)
     {
          fprintf(stderr, "WMFS Error: Cannot initialize font\n");
          xftfont = XftFontOpenName(dpy, screen, "sans-10");
     }
     fonth = (xftfont->ascent + xftfont->descent);
     barheight = fonth + 4;


     /* INIT CURSOR */
     cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
     cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
     cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

     /* INIT MODIFIER */
     modmap = XGetModifierMapping(dpy);
     for(i = 0; i < 8; i++)
          for(j = 0; j < modmap->max_keypermod; ++j)
               if(modmap->modifiermap[i * modmap->max_keypermod + j]
                  == XKeysymToKeycode(dpy, XK_Num_Lock))
                    numlockmask = (1 << i);
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

     /* INIT BAR / BUTTON */
     bary = (conf.bartop) ? 0 : mh - barheight;
     bar = bar_create(0, bary, mw, barheight, 0, conf.colors.bar, False);
     XMapRaised(dpy, bar->win);
     strcpy(bartext, "WMFS-" WMFS_VERSION);
     updatebutton(False);
     updatebar();

     /* INIT STUFF */
     XSetErrorHandler(errorhandler);
     grabkeys();

     return;
}

Bool
ishide(Client *c)
{
     int i;

     for(i = 0; i < conf.ntag + 1; ++i)
          if(c->tag == i && seltag == i)
               return False;
     return True;
}

void
uicb_killclient(uicb_t cmd)
{
     XEvent ev;

     if(!sel)
          return;

     ev.type = ClientMessage;
     ev.xclient.window = sel->win;
     ev.xclient.message_type = wm_atom[WMProtocols];
     ev.xclient.format = 32;
     ev.xclient.data.l[0] = wm_atom[WMDelete];
     ev.xclient.data.l[1] = CurrentTime;
     XSendEvent(dpy, sel->win, False, NoEventMask, &ev);

     return;
}

void
mainloop(void)
{
     fd_set fd;
     char sbuf[sizeof bartext], *p;
     int len, r, offset = 0;
     Bool readstdin = True;

     len = sizeof bartext - 1;
     sbuf[len] = bartext[len] = '\0';

     while(!exiting)
     {
          FD_ZERO(&fd);
          if(readstdin)
               FD_SET(STDIN_FILENO, &fd);
          FD_SET(ConnectionNumber(dpy), &fd);
          if(select(ConnectionNumber(dpy) + 1, &fd, NULL, NULL, NULL) == -1)
               fprintf(stderr, "WMFS Warning: Select failed\n");
          if(FD_ISSET(STDIN_FILENO, &fd))
          {
               if((r = read(STDIN_FILENO, sbuf + offset, len - offset)))
               {
                    for(p = sbuf + offset; r > 0; ++p, --r, ++offset)
                    {
                         if(*p == '\n')
                         {
                              *p = '\0';
                              strncpy(bartext, sbuf, len);
                              p += r - 1;
                              for(r = 0; *(p - r) && *(p - r) != '\n'; ++r);
                              offset = r;
                              if(r)
                                   memmove(sbuf, p - r + 1, r);
                              break;
                         }
                    }
               }
               else
               {
                    strncpy(bartext, sbuf, strlen(sbuf));
                    readstdin = False;
               }
               updatebar();
          }
          while(XPending(dpy))
          {
               XNextEvent(dpy, &event);
               getevent();
          }
     }

     return;
}

void
mapclient(Client *c)
{
     if(!c)
          return;
     XMapWindow(dpy, c->win);
     XMapSubwindows(dpy, c->win);
     if(conf.ttbarheight)
     {
          XMapWindow(dpy, c->tbar->win);
          bar_refresh(c->tbar);
          if(conf.ttbarheight > 5)
               XMapWindow(dpy, c->button);
     }

     return;
}

void
manage(Window w, XWindowAttributes *wa)
{
     Client *c, *t = NULL;
     Window trans;
     Status rettrans;
     XWindowChanges winc;

     c = emalloc(1, sizeof(Client));
     c->win = w;
     c->x = wa->x;
     c->y = wa->y + conf.ttbarheight + barheight;
     c->w = wa->width;
     c->h = wa->height;
     c->tag = seltag;

     /* Create titlebar & button */
     if(conf.ttbarheight)
     {


          c->tbar = bar_create(c->x, c->y - conf.ttbarheight,
                               c->w, c->h,conf.borderheight,
                               conf.colors.bar, True);

          /* Basic window for close button... */
          if(conf.ttbarheight > 5)
               c->button = XCreateSimpleWindow(dpy, root, BUTX(c->x, c->w), BUTY(c->y),
                                               ((BUTH) ? BUTH : 1), ((BUTH) ? BUTH : 1),
                                               1, conf.colors.button_border,
                                               conf.colors.button);
     }

     XConfigureWindow(dpy, w, CWBorderWidth, &winc);
     setborder(w, conf.colors.bordernormal);
     grabbuttons(c, False);
     XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask
                  | PropertyChangeMask | StructureNotifyMask);
     setsizehints(c);
     updatetitlebar(c);
     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);
     if(t)
          c->tag = t->tag;
     if(!c->free)
          c->free = (rettrans == Success) || c->hint;
     else
          raiseclient(c);

     attach(c);
     XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
     mapclient(c);
     setwinstate(c->win, NormalState);
     focus(c);
     arrange();

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
                    moveresize(c, c->x, c->y,
                               ((ev.xmotion.x - ocx <= 0) ? 1 : ev.xmotion.x - ocx),
                               ((ev.xmotion.y - ocy <= 0) ? 1 : ev.xmotion.y - ocy), True);
               /* Move */
               else
                    moveresize(c,
                               (ocx + (ev.xmotion.x - x)),
                               (ocy + (ev.xmotion.y - y)),
                               c->w, c->h, True);
          }
          else if(ev.type == Expose)
               expose(ev);
     }

     return;
}

void
moveresize(Client *c, int x, int y, int w, int h, bool r)
{
     if(!c)
          return;
     /* Resize hints {{{ */
     if(r)
     {
          /* minimum possible */
          if (w < 1)
               w = 1;
          if (h < 1)
               h = 1;
          /* base */
          w -= c->basew;
          h -= c->baseh;
          /* aspect */
          if (c->minay > 0 && c->maxay > 0
              && c->minax > 0 && c->maxax > 0)
          {
               if (w * c->maxay > h * c->maxax)
                    w = h * c->maxax / c->maxay;
               else if (w * c->minay < h * c->minax)
                    h = w * c->minay / c->minax;
          }
          /* incremental */
          if(c->incw)
               w -= w % c->incw;
          if(c->inch)
               h -= h % c->inch;
          /* base dimension */
          w += c->basew;
          h += c->baseh;

          if(c->minw > 0 && w < c->minw)
               w = c->minw;
          if(c->minh > 0 && h < c->minh)
               h = c->minh;
          if(c->maxw > 0 && w > c->maxw)
               w = c->maxw;
          if(c->maxh > 0 && h > c->maxh)
               h = c->maxh;
          if(w <= 0 || h <= 0)
               return;
     }
     /* }}} */

     c->max = False;
     if(c->x != x || c->y != y
        || c->w != w || c->h != h)
     {
          c->x = x; c->y = y;
          c->w = w; c->h = h;

          XMoveResizeWindow(dpy, c->win, x, y, w ,h);

          if(conf.ttbarheight)
          {
               bar_moveresize(c->tbar, x, y - conf.ttbarheight, w, conf.ttbarheight);
               if(conf.ttbarheight > 5)
                    XMoveWindow(dpy, c->button, BUTX(x, w), BUTY(y));
          }
          updatetitlebar(c);
          XSync(dpy, False);
     }

     return;
}

void
uicb_quit(uicb_t cmd)
{
     exiting = True;

     return;
}

void
raiseclient(Client *c)
{
     if(!c)
          return;
     XRaiseWindow(dpy, c->win);

     if(conf.ttbarheight)
     {
          XRaiseWindow(dpy, c->tbar->win);
          if(conf.ttbarheight > 5)
               XRaiseWindow(dpy, c->button);
          updatetitlebar(c);
     }

     return;
}

/* scan all the client who was in X before wmfs */
void
scan(void)
{
     uint i, num;
     Window *wins, d1, d2;
     XWindowAttributes wa;

     wins = NULL;
     if(XQueryTree(dpy, root, &d1, &d2, &wins, &num))
     {
          for(i = 0; i < num; i++)
          {
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

     arrange();

     return;
}

void
setborder(Window win, int color)
{
     XSetWindowBorder(dpy, win, color);
     XSetWindowBorderWidth(dpy, win, conf.borderheight);

     return;
}

void
setwinstate(Window win, long state)
{
     long data[] = {state, None};

     XChangeProperty(dpy, win, wm_atom[WMState], wm_atom[WMState], 32,
                     PropModeReplace, (unsigned char *)data, 2);

     return;
}

void
setsizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize) || !size.flags)
             size.flags = PSize;
        /* base */
	if(size.flags & PBaseSize)
        {
             c->basew = size.base_width;
             c->baseh = size.base_height;
        }
	else if(size.flags & PMinSize)
        {
             c->basew = size.min_width;
             c->baseh = size.min_height;
	}
	else
             c->basew = c->baseh = 0;
        /* inc */
	if(size.flags & PResizeInc)
        {
             c->incw = size.width_inc;
             c->inch = size.height_inc;
	}
	else
             c->incw = c->inch = 0;
        /* max */
	if(size.flags & PMaxSize)
        {
             c->maxw = size.max_width;
             c->maxh = size.max_height;
	}
	else
             c->maxw = c->maxh = 0;
        /* min */
	if(size.flags & PMinSize)
        {
             c->minw = size.min_width;
             c->minh = size.min_height;
	}
	else if(size.flags & PBaseSize)
        {
             c->minw = size.base_width;
             c->minh = size.base_height;
	}
	else
             c->minw = c->minh = 0;
        /* aspect */
	if(size.flags & PAspect)
        {
             c->minax = size.min_aspect.x;
             c->maxax = size.max_aspect.x;
             c->minay = size.min_aspect.y;
             c->maxay = size.max_aspect.y;
	}
	else
             c->minax = c->maxax = c->minay = c->maxay = 0;
        c->hint = (c->maxw && c->minw && c->maxh && c->minh
                   && c->maxw == c->minw && c->maxh == c->minh);

        return;
}

void
unhide(Client *c)
{
     if(!c)
          return;
     XMoveWindow(dpy, c->win, c->x, c->y);
     if(conf.ttbarheight)
     {
          bar_moveresize(c->tbar, c->x, c->y - conf.ttbarheight, c->w, conf.ttbarheight);
          if(conf.ttbarheight > 5)
               XMoveWindow(dpy, c->button, BUTX(c->x, c->w), BUTY(c->y));
     }
     //mapclient(c);
     setwinstate(c->win, NormalState);
     c->hide = False;

     return;
}

void
unmanage(Client *c)
{
     XGrabServer(dpy);
     XSetErrorHandler(errorhandlerdummy);
     sel = ((sel == c) ? ((c->next) ? c->next : NULL) : NULL);
     if(sel && sel->tag == seltag)
          selbytag[seltag] = sel;
     else
          selbytag[seltag] = NULL;
     detach(c);
     if(conf.ttbarheight)
     {
          bar_delete(c->tbar);
          if(conf.ttbarheight > 5)
          {
               XUnmapWindow(dpy, c->button);
               XDestroyWindow(dpy, c->button);
          }
     }
     setwinstate(c->win, WithdrawnState);
     free(c);
     free(c->tbar);
     XSync(dpy, False);
     XUngrabServer(dpy);
     arrange();

     return;
}

void
unmapclient(Client *c)
{
     if(!c)
          return;
     XUnmapWindow(dpy, c->win);
     if(conf.ttbarheight)
     {
          XUnmapWindow(dpy, c->tbar->win);
          if(conf.ttbarheight > 5)
               XUnmapWindow(dpy, c->button);
     }
     XUnmapSubwindows(dpy, c->win);

     return;
}

int
main(int argc, char **argv)
{
     int i;

     static struct option long_options[] = {

          {"help",	 	0, NULL, 'h'},
          {"info",		0, NULL, 'i'},
          {"version",     	0, NULL, 'v'},
          {NULL,		0, NULL, 0}
     };

     while ((i = getopt_long(argc, argv, "hvi", long_options, NULL)) != -1)
     {
          switch (i)
          {
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
                      "   - Martin Duquesnoy (code)\n"
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

     if(!(dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "WMFS: cannot open X server.\n");
          exit(EXIT_FAILURE);
     }

     /* Let's Go ! */
     init_conf();
     init();
     scan();
     mainloop();

     /* Exiting WMFS :'( */
     XftFontClose(dpy, xftfont);
     XFreeCursor(dpy, cursor[CurNormal]);
     XFreeCursor(dpy, cursor[CurMove]);
     XFreeCursor(dpy, cursor[CurResize]);
     bar_delete(bar);
     if(conf.nbutton)
          for(i = 0; i < conf.nbutton; ++i)
               bar_delete(conf.barbutton[i].bw);
     free(conf.barbutton);
     free(keys);
     XSync(dpy, False);
     XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);

     XCloseDisplay(dpy);

     return 0;
}

