/*
*      wmfs.c
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

void
arrange(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c))
               unhide(c);
          else
               hide(c);

     focus(NULL);
     tags[seltag].layout.func();
     updatebar();
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

void
detach(Client *c)
{
     if(c->prev) c->prev->next = c->next;
     if(c->next) c->next->prev = c->prev;
     if(c == clients) clients = c->next;
     c->next = c->prev = NULL;
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
     return(1);
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
     if(sel && sel != c)
     {
          grabbuttons(sel, False);
          setborder(sel->win, conf.colors.bordernormal);
          if(conf.ttbarheight)
               setborder(sel->tbar, conf.colors.bordernormal);
     }

     if(c)
          grabbuttons(c, True);

     sel = c;

     if(c)
     {
          setborder(c->win, conf.colors.borderfocus);
          if(conf.ttbarheight)
               setborder(sel->tbar, conf.colors.borderfocus);
          if(conf.raisefocus)
               raiseclient(c);
          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
          updatetitle(c);
     }
     else
          XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
     return;
}

void
freelayout(void)
{
     Client *c;

     tags[seltag].layout.func = freelayout;

     for(c = clients; c; c = c->next)
     {
          if(!ishide(c))
          {
               if(c->max || c->tile)
                    moveresize(c, c->ox, c->oy, c->ow, c->oh, 1);
               c->max = c->tile = False;
          }
     }
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

char*
getlayoutsym(int tag)
{
     if(tags[tag].layout.func == freelayout)
          return conf.layouts.free;
     else if(tags[tag].layout.func == tile)
          return conf.layouts.tile;
     else if(tags[tag].layout.func == maxlayout)
          return conf.layouts.max;

     return NULL;
}

Client*
gettbar(Window w)
{
     Client *c;

     for(c = clients; c && c->tbar != w; c = c->next);
     return c;
}

void
grabbuttons(Client *c, Bool focused)
{
     int i;

     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     if(conf.ttbarheight)
     {
          XUngrabButton(dpy, AnyButton, AnyModifier, c->tbar);
          XUngrabButton(dpy, AnyButton, AnyModifier, c->button);
     }

     if(focused)
     {
          /* Window */
          XGrabButton(dpy, Button1, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button1, ALT|LockMask, c->win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button2, ALT|LockMask, c->win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT, c->win, 0, ButtonMask,GrabModeAsync,GrabModeSync, None, None);
          XGrabButton(dpy, Button3, ALT|LockMask, c->win, False, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          if(conf.ttbarheight)
          {
               /* Titlebar */
               XGrabButton(dpy, Button1, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, Button2, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, Button3, AnyModifier, c->tbar, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
               /* Titlebar Button */
               XGrabButton(dpy, Button1, AnyModifier, c->button, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, Button3, AnyModifier, c->button, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
          }
          /* Bar Button */
          for(i=0; i< conf.nbutton; ++i)
               XGrabButton(dpy, Button1, AnyModifier, conf.barbutton[i].win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
     }
     else
     {
          XGrabButton(dpy, AnyButton, AnyModifier, c->win, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          if(conf.ttbarheight)
          {
               XGrabButton(dpy, AnyButton, AnyModifier, c->tbar, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(dpy, AnyButton, AnyModifier, c->button, 0, ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          }
          for(i=0; i< conf.nbutton; ++i)
               XGrabButton(dpy, Button1, AnyModifier, conf.barbutton[i].win, 0, ButtonMask,GrabModeAsync, GrabModeSync, None, None);
     }
}

void
grabkeys(void)
{
     unsigned int i;
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
          XMoveWindow(dpy, c->tbar, c->x, c->y+mh*2);
          XMoveWindow(dpy, c->button, c->x, c->y+mh*2);
     }
     setwinstate(c->win, IconicState);
     c->hide = True;
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
     seltag = 1;
     taglen[0] = 3;

     /* INIT FONT */
     font = XLoadQueryFont(dpy, conf.font);
     if(!font)
     {
          fprintf(stderr, "XLoadQueryFont: failed loading font '%s'\n", conf.font);
          exit(0);
     }
     XSetFont(dpy, gc, font->fid);
     fonth = (font->ascent + font->descent) - 1;
     barheight = fonth + 3;
     fonty = (font->ascent + font->descent) / 2;
     /* init button font */
     font_b = XLoadQueryFont(dpy, conf.buttonfont);
     if(!font_b)
     {
          fprintf(stderr, "XLoadQueryFont: failed loading button font '%s'\n", conf.buttonfont);
          exit(0);
     }

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
     dr = XCreatePixmap(dpy, root, DisplayWidth(dpy, screen), barheight, DefaultDepth(dpy, screen));
     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     at.event_mask = ButtonPressMask | ExposureMask;
     bar = XCreateWindow(dpy, root, 0, 0, mw, barheight, 0, DefaultDepth(dpy, screen),
                         CopyFromParent, DefaultVisual(dpy, screen),
                         CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);
     XMapRaised(dpy, bar);
     updatebar();
     updatebutton(0);

     /* INIT STUFF */
     XSetErrorHandler(errorhandler);
     grabkeys();

     return;
}

Bool
ishide(Client *c)
{
     int i;

     for(i = 0; i < conf.ntag+1; ++i)
          if(c->tag == i && seltag == i)
               return False;
     return True;
}

void
keymovex(char *cmd)
{
     int tmp;

     if(sel && cmd && !ishide(sel) && !sel->max && !sel->tile)
     {
          tmp = sel->x + atoi(cmd);
          moveresize(sel, tmp, sel->y, sel->w, sel->h, 1);
     }
     return;
}

void
keymovey(char *cmd)
{
     int tmp;

     if(sel && cmd && !ishide(sel) && !sel->max && !sel->tile)
     {
          tmp = sel->y + atoi(cmd);
          moveresize(sel, sel->x, tmp, sel->w, sel->h, 1);
     }
     return;
}

void
keyresize(char *cmd)
{
     int temph = 0, tempw = 0,
          modh = 0, modw = 0, tmp = 0;

     if(sel && !ishide(sel) && !sel->max && !sel->tile)
     {
          switch(cmd[1])
          {
          case 'h': tmp = (cmd[0] == '+') ? 5 : -5; modh = tmp; break;
          case 'w': tmp = (cmd[0] == '+') ? 5 : -5; modw = tmp; break;
          }
          temph = sel->h + modh;
          tempw = sel->w + modw;
          temph = (temph < 10) ? 10 : temph;
          tempw = (tempw < 10) ? 10 : tempw;
          moveresize(sel, sel->x, sel->y, tempw, temph, 1);
     }
     return;
}

void
killclient(char *cmd)
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
layoutswitch(char *cmd)
{
     void (*tmpfunc)(void);

     if(cmd[0] == '+')
     {
          if(tags[seltag].layout.func == freelayout)
               tmpfunc = tile;
          else if(tags[seltag].layout.func == tile)
               tmpfunc = maxlayout;
          else if(tags[seltag].layout.func == maxlayout)
               tmpfunc = freelayout;
     }
     else if(cmd[0] == '-')
     {
          if(tags[seltag].layout.func == freelayout)
               tmpfunc = maxlayout;
          else if(tags[seltag].layout.func == tile)
               tmpfunc = freelayout;
          else if(tags[seltag].layout.func == maxlayout)
               tmpfunc = tile;
     }

     tags[seltag].layout.func = tmpfunc;
     arrange();
     return;
}

void
lowerclient(Client *c)
{
     if(!c)
          return;
     if(conf.ttbarheight)
     {
          XLowerWindow(dpy,c->button);
          XLowerWindow(dpy,c->tbar);
     }
     XLowerWindow(dpy,c->win);
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
          XMapWindow(dpy, c->tbar);
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

     c = emallocz(sizeof(Client));
     c->win = w;
     c->x = wa->x;
     c->y = wa->y + conf.ttbarheight + barheight;
     c->w = wa->width;
     c->h = wa->height;
     c->tag = seltag;

     if(conf.ttbarheight)
     {
          c->tbar = XCreateSimpleWindow(dpy, root, c->x, c->y - conf.ttbarheight,
                                        c->w, conf.ttbarheight, conf.borderheight,
                                        conf.colors.bordernormal, conf.colors.bar);
          XSelectInput(dpy, c->tbar, ExposureMask | EnterWindowMask);

          c->button = XCreateSimpleWindow(dpy, root, BUTX(c->x, c->w),
                                          BUTY(c->y), BUTH, BUTH,
                                          1, conf.colors.bordernormal,
                                          conf.colors.borderfocus);
     }

     XConfigureWindow(dpy, w, CWBorderWidth, &winc);
     setborder(w, conf.colors.bordernormal);
     grabbuttons(c, False);
     XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask
                  | PropertyChangeMask | StructureNotifyMask);
     setsizehints(c);
     updatetitle(c);
     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);
     if(t)
          c->tag = t->tag;
     if(!c->free)
          c->free = (rettrans == Success) || c->hint;
     if(c->free)
          raiseclient(c);

     attach(c);
     XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
     mapclient(c);
     setwinstate(c->win, NormalState);
     arrange();
     return;
}

void
maxlayout(void)
{
     Client *c;

     tags[seltag].layout.func = maxlayout;

     for(c = nexttiled(clients); c; c = nexttiled(c->next))
     {
          c->tile = False;
          c->ox = c->x;

          c->oy = c->y;
          c->ow = c->w;
          c->oh = c->h;
          moveresize(c, 0,
                     conf.ttbarheight + barheight,
                     (mw-(conf.borderheight * 2)),
                     (mh-(conf.borderheight * 2) - conf.ttbarheight - barheight), 0);
          c->max = True;
     }
     return;
}


/* If the type is 0, this function will move, else,
   this will resize */
void
mouseaction(Client *c, int x, int y, int type)
{
     int  ocx, ocy;
     XEvent ev;

     if(c->max || c->tile)
          return;

     ocx = c->x;
     ocy = c->y;
     if(XGrabPointer(dpy, root, 0, MouseMask, GrabModeAsync, GrabModeAsync,
                     None, cursor[((type) ?CurResize:CurMove)], CurrentTime) != GrabSuccess) return;
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
               return;
          }
          else if(ev.type == MotionNotify)
          {
               XSync(dpy, False);
               /* Resize */
               if(type)
                    moveresize(c, c->x, c->y,
                               ((ev.xmotion.x - ocx <= 0) ? 1 : ev.xmotion.x - ocx),
                               ((ev.xmotion.y - ocy <= 0) ? 1 : ev.xmotion.y - ocy), 1);
               /* Move */
               else
                    moveresize(c,
                               (ocx + (ev.xmotion.x - x)),
                               (ocy + (ev.xmotion.y - y)),
                               c->w, c->h, 1);

               /* for don't pass on the bar */
               if(c->y < barheight + conf.ttbarheight - 5)
               {
                    moveresize(c, c->x, barheight+conf.ttbarheight, c->w, c->h, 1);
                    XUngrabPointer(dpy, CurrentTime);
                    return;
               }
          }
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

          if((y - conf.ttbarheight) <= barheight)
               y = barheight+conf.ttbarheight;

          XMoveResizeWindow(dpy, c->win, x, y, w ,h);

          if(conf.ttbarheight)
          {
               XMoveResizeWindow(dpy, c->tbar, x, y - conf.ttbarheight, w, conf.ttbarheight);
               XMoveWindow(dpy, c->button, BUTX(x, w),  BUTY(y));
          }
          updateall();
          XSync(dpy, False);
     }
     return;
}

Client*
nexttiled(Client *c)
{
     for(; c && (c->free || ishide(c)); c = c->next);
     return c;
}

void
quit(char *cmd) {
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
          XRaiseWindow(dpy, c->tbar);
          XRaiseWindow(dpy, c->button);
     }
     return;
}

/* scan all the client who was in X before wmfs */
void
scan(void)
{
     unsigned int i, num;
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
setwinstate(Window win, long state) {
     long data[] = {state, None};

     XChangeProperty(dpy, win, wm_atom[WMState], wm_atom[WMState], 32,
                     PropModeReplace, (unsigned char *)data, 2);
}

void
set_mwfact(char *cmd)
{
     double c;

     if(!(sscanf(cmd, "%lf", &c)))
        return;
     if(tags[seltag].mwfact + c > 0.95
        || tags[seltag].mwfact + c < 0.05
        || tags[seltag].layout.func != tile)
          return;
     tags[seltag].mwfact += c;
     arrange();
     return;
}

void
set_nmaster(char *cmd)
{
     int n = atoi(cmd);

     if(tags[seltag].nmaster + n == 0)
          return;
     tags[seltag].nmaster += n;
     arrange();
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
        /* nax */
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
}

/* if cmd is +X or -X, this is just switch
   else {1, 2.. 9} it's go to the wanted tag. */
void
tag(char *cmd)
{
     int tmp = atoi(cmd);

     if(!tmp)
          tmp = 1;

     if(cmd[0] == '+' || cmd[0] == '-')
     {
          if(tmp + seltag < 1
             || tmp + seltag > conf.ntag)
               return;
          seltag += tmp;
     }
     else
     {
          if(tmp == seltag)
               return;
          seltag = tmp;
     }

     arrange();
     return;
}

void
tagtransfert(char *cmd)
{
     int n = atoi(cmd);

     if(!sel)
          return;

     if(!n)
          n = 1;

     sel->tag = n;
     arrange();
}

void
tile(void)
{
     unsigned int i, n, x, y, w, h, ww, hh, th;
     unsigned int barto, bord, mwf, nm;
     Client *c;

     bord    =  conf.borderheight * 2;
     barto   =  conf.ttbarheight + barheight;
     mwf     =  tags[seltag].mwfact * mw;
     nm      =  tags[seltag].nmaster;

     tags[seltag].layout.func = tile;

     /* count all the "can-be-tiled" client */
     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), n++);
     if(n == 0)
          return;

     /* window geoms */
     hh = ((n <= nm) ? mh / (n > 0 ? n : 1) : mh / nm) - bord*2;
     ww = (n  <= nm) ? mw : mwf;
     th = (n  >  nm) ? mh / (n - nm) : 0;
     if(n > nm && th < barheight)
          th = mh;

     x = 0;
     y = barto;

     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), i++)
     {
          c->max = False;
          c->tile = True;
          c->ox = c->x; c->oy = c->y;
          c->ow = c->w; c->oh = c->h;
          /* MASTER CLIENT */
          if(i < nm)
          {
               y = barto + i * hh;
               w = ww - bord;
               h = hh;
               /* remainder */
               if(i + 1 == (n < nm ? n : nm))
                    h = (mh - hh*i) - barheight ;
               h -= bord + conf.ttbarheight;
          }
          /* TILE CLIENT */
          else
          {
               if(i == nm)
               {
                    y = barto;
                    x += ww;
               }
               w = mw - ww - bord;
               /* remainder */
               if(i + 1 == n)
                    h = (barto + mh) - y - (bord + barto);
               else
                    h = th - (bord + conf.ttbarheight) - bord*2;
          }
          moveresize(c, x, y, w, h, 0);
          if(n > nm && th != mh)
               y = c->y + c->h + bord + conf.ttbarheight;
     }
     return;
}

void
tile_switch(char *cmd)
{
     Client *c;

     if(!sel || sel->hint || !sel->tile)
          return;
     if((c = sel) == nexttiled(clients))
          if(!(c = nexttiled(c->next)))
               return;
     detach(c);
     attach(c);
     focus(c);
     arrange();
}

void
togglemax(char *cmd)
{
     if(!sel || ishide(sel) || sel->hint)
          return;
     if(!sel->max)
     {
          sel->ox = sel->x; sel->oy = sel->y;
          sel->ow = sel->w; sel->oh = sel->h;
          moveresize(sel, 0,
                     conf.ttbarheight + barheight,
                     (mw-(conf.borderheight * 2)),
                     (mh-(conf.borderheight * 2)- conf.ttbarheight - barheight), 0);
          raiseclient(sel);
          sel->max = True;
     }
     else if(sel->max)
     {
          moveresize(sel, sel->ox, sel->oy, sel->ow, sel->oh, 0);
          sel->max = False;
     }
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
          XMoveWindow(dpy, c->tbar, c->x, (c->y - conf.ttbarheight));
          XMoveWindow(dpy, c->button, BUTX(c->x, c->w), BUTY(c->y));
     }
     setwinstate(c->win, NormalState);
     c->hide = False;
}

void
unmanage(Client *c)
{
     XGrabServer(dpy);
     XSetErrorHandler(errorhandlerdummy);
     sel = ((sel == c) ? ((c->next) ? c->next : NULL) : NULL);
     detach(c);
     if(conf.ttbarheight)
     {
          XUnmapWindow(dpy, c->tbar);
          XDestroyWindow(dpy, c->tbar);
          XUnmapWindow(dpy, c->button);
          XDestroyWindow(dpy, c->button);
     }
     setwinstate(c->win, WithdrawnState);
     free(c);
     XSync(dpy, False);
     XUngrabServer(dpy);
     arrange();
     return;
}

void
updateall(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c))
               updatetitle(c);
}

void
updatebar(void)
{
     int  i ,j;
     char buf[conf.ntag][sizeof(char)];
     char *p = malloc(sizeof(char));
     tm = localtime(&lt);
     lt = time(NULL);

     XSetForeground(dpy, gc, conf.colors.bar);
     XFillRectangle(dpy, dr, gc, 0, 0, mw, barheight);

     for(i = 0; i < conf.ntag; ++i)
     {
          /* Make the tags string */
          ITOA(p, clientpertag(i+1));
          sprintf(buf[i], "%s<%s> ", tags[i].name, (clientpertag(i+1)) ? p : "");
          taglen[i+1] = (taglen[i] + fonty * (strlen(tags[i].name) +
                                              strlen(buf[i]) - strlen(tags[i].name)) + fonty) - 2;
          /* Rectangle for the tag background */
          XSetForeground(dpy, gc, (i+1 == seltag) ? conf.colors.tagselbg : conf.colors.bar);
          XFillRectangle(dpy, dr, gc, taglen[i] - 3, 0, (strlen(buf[i])*fonty) -2, barheight);

          /* Draw tag */
          XSetForeground(dpy, gc, (i+1 == seltag) ? conf.colors.tagselfg : conf.colors.text);
          XDrawString(dpy, dr, gc, taglen[i], fonth, buf[i], strlen(buf[i]));
     }

     /* Draw layout symbol */
     XSetForeground(dpy, gc, conf.colors.layout_bg);
     XFillRectangle(dpy, dr, gc, taglen[conf.ntag] - 5, 0,
                    (strlen(getlayoutsym(seltag))*fonty) + 1, barheight);
     XSetForeground(dpy, gc, conf.colors.layout_fg);
     XDrawString(dpy, dr, gc, taglen[conf.ntag] - 4,
                 fonth,
                 getlayoutsym(seltag),
                 strlen(getlayoutsym(seltag)));

     /* Draw status */
     sprintf(bartext,"mwfact: %.2f  nmaster: %i - %02i:%02i",
             tags[seltag].mwfact,
             tags[seltag].nmaster,
             tm->tm_hour,
             tm->tm_min);

     j = strlen(bartext);
     XSetForeground(dpy, gc, conf.colors.text);
     XDrawString(dpy, dr, gc, mw - j * fonty, fonth-1, bartext ,j);
     XDrawLine(dpy, dr, gc, mw- j * fonty-5, 0, mw - j * fonty-5, barheight);

     XCopyArea(dpy, dr, bar, gc, 0, 0, mw, barheight, 0, 0);
     XSync(dpy, False);

     /* Update Bar Buttons */
     updatebutton(1);
     free(p);
}

/* if c is 0, you can execute this function for the first time
 * else the button is just updated */
void
updatebutton(Bool c)
{
     int i, j, p, x, pm = 0;
     XSetWindowAttributes at;
     int y = 3;
     int h = barheight - 5;
     int fonth_l = fonth - 3;

     at.override_redirect = 1;
     at.background_pixmap = ParentRelative;
     at.event_mask = ButtonPressMask | ExposureMask;

     j = taglen[conf.ntag] + ((strlen(getlayoutsym(seltag))*fonty) + 2);

     XSetFont(dpy, gc, font_b->fid);

     for(i = 0; i < conf.nbutton; ++i)
     {
          p = strlen(conf.barbutton[i].text);
          if(!conf.barbutton[i].x)
          {
               if(i)
                    pm += strlen(conf.barbutton[i-1].text) * fonty+1;
               x = (!i) ? j : j + pm;
          }
          else
               x = conf.barbutton[i].x;

          if(!c)
          {
               conf.barbutton[i].win = XCreateWindow(dpy, root, x, y, p*fonty+1, h,
                                                     0, DefaultDepth(dpy, screen),
                                                     CopyFromParent, DefaultVisual(dpy, screen),
                                                     CWOverrideRedirect | CWBackPixmap | CWEventMask, &at);
               XSetWindowBackground(dpy, conf.barbutton[i].win, conf.barbutton[i].bg_color);
               XMapRaised(dpy, conf.barbutton[i].win);
               XSetForeground(dpy, gc, conf.barbutton[i].fg_color);
               XDrawString(dpy, conf.barbutton[i].win, gc, 1, fonth_l, conf.barbutton[i].text, p);
          }
          else
          {
               if(!conf.barbutton[i].win)
                    return;
               XSetForeground(dpy, gc, conf.barbutton[i].fg_color);
               XMoveWindow(dpy, conf.barbutton[i].win, x, y);
               XDrawString(dpy, conf.barbutton[i].win, gc, 1, fonth_l,
                           conf.barbutton[i].text, strlen(conf.barbutton[i].text));
          }
     }
     XSetFont(dpy, gc, font->fid);
     XSync(dpy, False);
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
          XUnmapWindow(dpy, c->tbar);
          XUnmapWindow(dpy, c->button);
     }
     XUnmapSubwindows(dpy, c->win);
     return;
}

void
updatetitle(Client *c)
{
     XFetchName(dpy, c->win, &(c->title));
     if(!c->title)
          c->title = strdup("WMFS");
     if(conf.ttbarheight > 10)
     {
          XClearWindow(dpy, c->tbar);
          XSetForeground(dpy, gc, conf.colors.text);
          XDrawString(dpy, c->tbar, gc, 3,
                      ((fonth-2) + ((conf.ttbarheight - fonth) / 2)),
                      c->title, strlen(c->title));
     }
     return;
}

void
wswitch(char *cmd)
{
     Client *c;

     if(!sel || ishide(sel))
          return;
     if(cmd[0] == '+')
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
     else if(cmd[0] == '-')
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
     updateall();
     return;
}

int
main(int argc,char **argv)
{
     dpy = XOpenDisplay(NULL);
     int i;

     static struct option long_options[] = {

          {"help",	 	0, NULL, 'h'},
          {"info",		0, NULL, 'i'},
          {"version",     	0, NULL, 'v'},
          {NULL,		0, NULL, 0}
     };

     while ((i = getopt_long (argc, argv, "hvi", long_options, NULL)) != -1)
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

     if(!dpy)
     {
          printf("WMFS: cannot open X server\n");
          exit(1);
     }

     /* Let's Go ! */
     init_conf();
     init();
     scan();
     updatebar();

     while(!exiting)
     {
          getevent();
          updateall();
     }

     /* exiting WMFS :'( */
     XFreeFont(dpy, font);
     XFreeFont(dpy, font_b);
     XUngrabKey(dpy, AnyKey, AnyModifier, root);
     XFreeCursor(dpy, cursor[CurNormal]);
     XFreeCursor(dpy, cursor[CurMove]);
     XFreeCursor(dpy, cursor[CurResize]);
     XDestroyWindow(dpy, bar);
     XSync(dpy, False);
     XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);

     XCloseDisplay(dpy);
     return 0;
}

