/*
*      client.c
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

int
client_pertag(int tag)
{
     Client *c;
     int i = 0;

     for(c = clients; c; c = c->next)
          if(c->tag == tag)
               ++i;

     return i;
}

void
client_attach(Client *c)
{
     if(clients)
          clients->prev = c;
     c->next = clients;
     clients = c;

     return;
}

void
client_detach(Client *c)
{
     Client **cc;

     for(cc = &clients; *cc && *cc != c; cc = &(*cc)->next);
     *cc = c->next;

     return;
}

/* Fixed, need testing */
void
uicb_client_prev(uicb_t cmd)
{
     Client *c = NULL, *d;

     if(!sel || ishide(sel))
          return;

     for(d = clients; d != sel; d = d->next)
          if(!ishide(d))
               c = d;
     if(!c)
          for(; d; d = d->next)
               if(!ishide(d))
                    c = d;
     if(c)
     {
          client_focus(c);
          if(!c->tile)
               client_raise(c);
     }
     arrange();

     return;
}

void
uicb_client_next(uicb_t cmd)
{
     Client *c = NULL;

     if(!sel || ishide(sel))
          return;

     for(c = sel->next; c && ishide(c); c = c->next);
     if(!c)
          for(c = clients; c && ishide(c); c = c->next);
     if(c)
     {
          client_focus(c);
          if(!c->tile)
               client_raise(c);
     }
     arrange();

     return;
}

void
client_focus(Client *c)
{
     Client *cc;

     if(sel && sel != c)
     {
          grabbuttons(sel, False);
          XSetWindowBorder(dpy, sel->win, conf.client.bordernormal);
     }

     selbytag[seltag] = sel = c;

     if(c)
     {
          grabbuttons(c, True);
          XSetWindowBorder(dpy, c->win, conf.client.borderfocus);
          if(conf.raisefocus)
               client_raise(c);
          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
     }
     else
          XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);

     /* Update all visible titlebar */
     for(cc = clients; cc; cc = cc->next)
          if(!ishide(cc))
               titlebar_update(cc);

     return;
}

Client*
client_get(Window w)
{
     Client *c;

     for(c = clients; c && c->win != w; c = c->next);

     return c;
}

void
client_hide(Client *c)
{
     //XMoveWindow(dpy, c->win, c->geo.x + mw * 2, c->geo.y);
     //if(conf.titlebar.exist)
     //     XMoveWindow(dpy, c->tbar->win, c->geo.x + mw * 2, c->geo.y);
     client_unmap(c);
     setwinstate(c->win, IconicState);

     return;
}


Bool
ishide(Client *c)
{
     if(c->tag && c->tag == seltag)
          return False;
     return True;
}

void
uicb_client_kill(uicb_t cmd)
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
client_map(Client *c)
{
     if(!c)
          return;

     if(conf.titlebar.exist)
     {
          XMapWindow(dpy, c->tbar->win);
          bar_refresh(c->tbar);
     }
     XMapWindow(dpy, c->win);

     return;
}

void
client_manage(Window w, XWindowAttributes *wa)
{
     Client *c, *t = NULL;
     Window trans;
     Status rettrans;
     XWindowChanges winc;

     c = emalloc(1, sizeof(Client));
     c->win = w;
     c->geo.x = wa->x;
     c->geo.y = wa->y + sgeo.y + conf.titlebar.height;
     c->geo.width = wa->width;
     c->geo.height = wa->height;
     c->tag = seltag;
     c->border = conf.client.borderheight;

     /* Create titlebar */
     if(conf.titlebar.exist)
          titlebar_create(c);

     winc.border_width = c->border;
     XConfigureWindow(dpy, w, CWBorderWidth, &winc);
     XSetWindowBorder(dpy, w, conf.client.bordernormal);
     grabbuttons(c, False);
     XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask
                  | PropertyChangeMask | StructureNotifyMask);
     client_size_hints(c);
     titlebar_update(c);
     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);
     if(t)
          c->tag = t->tag;
     if(!c->free)
          c->free = (rettrans == Success) || c->hint;
     efree(t);

     client_attach(c);
     XMoveResizeWindow(dpy, c->win, c->geo.x, c->geo.y, c->geo.width, c->geo.height);
     client_map(c);
     client_raise(c);
     setwinstate(c->win, NormalState);
     client_focus(c);
     arrange();

     return;
}

void
client_moveresize(Client *c, XRectangle geo, bool r)
{
     if(!c)
          return;

     /* Resize hints {{{ */
     if(r)
     {
          /* minimum possible */
          if (geo.width < 1)
               geo.width = 1;
          if (geo.height < 1)
               geo.height = 1;

          /* base */
          geo.width -= c->basew;
          geo.height -= c->baseh;

          /* aspect */
          if (c->minay > 0 && c->maxay > 0
              && c->minax > 0 && c->maxax > 0)
          {
               if (geo.width * c->maxay > geo.height * c->maxax)
                    geo.width = geo.height * c->maxax / c->maxay;
               else if (geo.width * c->minay < geo.height * c->minax)
                    geo.height = geo.width * c->minay / c->minax;
          }

          /* incremental */
          if(c->incw)
               geo.width -= geo.width % c->incw;
          if(c->inch)
               geo.height -= geo.height % c->inch;

          /* base dimension */
          geo.width += c->basew;
          geo.height += c->baseh;

          if(c->minw > 0 && geo.width < c->minw)
               geo.width = c->minw;
          if(c->minh > 0 && geo.height < c->minh)
               geo.height = c->minh;
          if(c->maxw > 0 && geo.width > c->maxw)
               geo.width = c->maxw;
          if(c->maxh > 0 && geo.height > c->maxh)
               geo.height = c->maxh;
          if(geo.width <= 0 || geo.height <= 0)
               return;
     }
     /* }}} */

     c->max = False;
     if(c->geo.x != geo.x
        || c->geo.y != geo.y
        || c->geo.width != geo.width
        || c->geo.height != geo.height)
     {
          c->geo = geo;

          XMoveResizeWindow(dpy, c->win, geo.x, geo.y,
                            geo.width, geo.height);

          if(conf.titlebar.exist)
               titlebar_update_position(c);

          titlebar_update(c);
          XSync(dpy, False);
     }

     return;
}

void
client_size_hints(Client *c)
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
client_raise(Client *c)
{
     if(!c || c->max || c->tile)
          return;

     if(conf.titlebar.exist)
     {
          XRaiseWindow(dpy, c->tbar->win);
          titlebar_update(c);
     }
     XRaiseWindow(dpy, c->win);

     return;
}

void
uicb_client_raise(uicb_t cmd)
{
     client_raise(sel);

     return;
}

void
client_unhide(Client *c)
{
     //XMoveWindow(dpy, c->win, c->geo.x, c->geo.y);
     //if(conf.titlebar.exist)
     //     titlebar_update_position(c);
     client_map(c);
     setwinstate(c->win, NormalState);

     return;
}

void
client_unmanage(Client *c)
{
     int i;
     Client *cc;

     XGrabServer(dpy);
     XSetErrorHandler(errorhandlerdummy);
     if(sel == c)
          client_focus(NULL);
     for(i = 0, cc = clients; cc; cc = cc->next, ++i)
          if(selbytag[i] == c)
               selbytag[i] = NULL;
     client_detach(c);
     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     setwinstate(c->win, WithdrawnState);
     XSync(dpy, False);
     XUngrabServer(dpy);
     if(conf.titlebar.exist)
          titlebar_delete(c);
     efree(c);
     arrange();

     return;
}

void
client_unmap(Client *c)
{
     if(!c)
          return;

     XUnmapWindow(dpy, c->win);
     if(conf.titlebar.exist)
          XUnmapWindow(dpy, c->tbar->win);

     return;
}
