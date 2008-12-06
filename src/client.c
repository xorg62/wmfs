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

/** Attach client in the client chain
 * \param c Client pointer
*/
void
client_attach(Client *c)
{
     if(clients)
          clients->prev = c;
     c->next = clients;
     clients = c;

     return;
}

/** Send a ConfigureRequest event to the Client
 * \param c Client pointer
*/
void
client_configure(Client *c)
{
     XConfigureEvent ev;

     ev.type               = ConfigureNotify;
     ev.event              = c->win;
     ev.window             = c->win;
     ev.x                  = c->geo.x;
     ev.y                  = c->geo.y;
     ev.width              = c->geo.width;
     ev.height             = c->geo.height;
     ev.above              = None;
     ev.border_width       = 0;
     ev.override_redirect  = 0;

     XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ev);

     return;
}

/** Detach a client to the client chain
 * \param c Client pointer
*/
void
client_detach(Client *c)
{
     Client **cc;

     for(cc = &clients; *cc && *cc != c; cc = &(*cc)->next);
     *cc = c->next;

     return;
}

/** Switch to the previous client
 * \param cmd uicb_t type unused
*/
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
          client_raise(c);
     }

     return;
}

/** Switch to the next client
 * \param cmd uicb_t type unused
*/
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
          client_raise(c);
     }

     return;
}

/** Set the focus to a client
 * \param c Client pointer
*/
void
client_focus(Client *c)
{
     if(sel && sel != c)
     {
          sel->colors.frame = conf.client.bordernormal;
          sel->colors.resizecorner = conf.client.resizecorner_normal;
          frame_update(sel);
          mouse_grabbuttons(sel, False);
     }

     sel = c;

     if(c)
     {
          c->colors.frame = conf.client.borderfocus;
          c->colors.resizecorner = conf.client.resizecorner_focus;
          frame_update(c);
          mouse_grabbuttons(c, True);
          if(conf.raisefocus)
               client_raise(c);
          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
     }
     else
          XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);

     return;
}

/* The following function have the same point :
 * find a client member with a Window {{{
 */

/* Get Client with a window */
/** Get a client->win with a window
 * \param w Window
 * \return The client
*/
     Client* client_gb_win(Window w)
     {
          Client *c;

          for(c = clients; c && c->win != w; c = c->next);

          return c;
     }

/** Get a client->frame with a window
 * \param w Window
 * \return The client
*/
     Client* client_gb_frame(Window w)
     {
          Client *c;

          for(c = clients; c && c->frame != w; c = c->next);

          return c;
     }

/** Get a client->titlebar with a window
 * \param w Window
 * \return The client
*/
     Client* client_gb_titlebar(Window w)
     {
          Client *c;

          if(!TBARH)
               return NULL;

          for(c = clients; c && c->titlebar->win != w; c = c->next);

          return c;
     }

/** Get a client->resize with a window
 * \param w Window
 * \return The client
*/
     Client* client_gb_resize(Window w)
     {
          Client *c;

          for(c = clients; c && c->resize != w; c = c->next);

          return c;
     }

/* }}} */

/** Get a client name
 * \param c Client pointer
*/
void
client_get_name(Client *c)
{
     XFetchName(dpy, c->win, &(c->title));
     if(!c->title)
          c->title = strdup("WMFS");

     frame_update(c);

     return;
}

/** Hide a client (for tag switching)
 * \param c Client pointer
*/
void
client_hide(Client *c)
{
     CHECK(!c->hide);

     client_unmap(c);
     c->hide = True;
     setwinstate(c->win, IconicState);

     return;
}

/** Check if the client 'c' is hide
 * \param c Client pointer
 * \return True if the client is hide; False if not
*/
Bool
ishide(Client *c)
{
     if(c->tag && c->tag == seltag[screen_get_sel()])
          return False;
     return True;
}

/** Kill a client
 * \param c Client pointer
*/
void
client_kill(Client *c)
{
     XEvent ev;
     Atom *atom = NULL;
     int proto;
     Bool canbedel = 0;

     CHECK(c);

     if(XGetWMProtocols(dpy, c->win, &atom, &proto) && atom)
     {
          while(proto--)
               if(atom[proto] == wm_atom[WMDelete])
                    ++canbedel;
          XFree(atom);
          if(canbedel)
          {
               ev.type = ClientMessage;
               ev.xclient.window = c->win;
               ev.xclient.message_type = wm_atom[WMProtocols];
               ev.xclient.format = 32;
               ev.xclient.data.l[0] = wm_atom[WMDelete];
               ev.xclient.data.l[1] = CurrentTime;
               ev.xclient.data.l[2] = 0;
               ev.xclient.data.l[3] = 0;
               ev.xclient.data.l[4] = 0;
               XSendEvent(dpy, c->win, False, NoEventMask, &ev);
          }
          else
               XKillClient(dpy, c->win);
     }
     else
          XKillClient(dpy, c->win);

     return;
}

/** Kill the selected client
 * \param cmd uicb_t type unused
*/
void
uicb_client_kill(uicb_t cmd)
{
     CHECK(sel);

     client_kill(sel);

     return;
}

/** Map a client
 * \param c Client pointer
*/
void
client_map(Client *c)
{
     CHECK(c);

     XMapWindow(dpy, c->frame);
     XMapSubwindows(dpy, c->frame);
     if(TBARH)
     {
          barwin_map(c->titlebar);
          barwin_map_subwin(c->titlebar);
     }
     c->unmapped = False;

     return;
}

/** Manage a client with a window and his attributes
 * \param w Cient's futur Window
 * \param wa XWindowAttributes pointer, Window w attributes
*/
void
client_manage(Window w, XWindowAttributes *wa)
{
     Client *c, *t = NULL;
     Window trans;
     Status rettrans;
     XSetWindowAttributes at;
     int mx, my;

     c = emalloc(1, sizeof(Client));
     c->win = w;
     c->screen = screen_get_sel();

     if(conf.client.place_at_mouse)
     {
          int dint;
          uint duint;
          Window dw;

          XQueryPointer(dpy, root, &dw, &dw, &mx, &my, &dint, &dint, &duint);

          mx += BORDH;
          my += TBARH;

          if((MAXW - mx) < wa->width)
               mx -= wa->width + BORDH;

          if((MAXH - my) < wa->height)
               my -= wa->height + TBARH;
     }
     else
     {
          mx = wa->x + BORDH;
          my = wa->y + TBARH + INFOBARH;
     }

     c->ogeo.x = c->geo.x = mx;
     c->ogeo.y = c->geo.y = my;

     c->ogeo.width = c->geo.width = wa->width;
     c->ogeo.height = c->geo.height = wa->height;
     c->tag = seltag[c->screen];
     at.event_mask = PropertyChangeMask;

     frame_create(c);
     client_size_hints(c);
     XChangeWindowAttributes(dpy, c->win, CWEventMask, &at);
     mouse_grabbuttons(c, False);
     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);
     if(t) c->tag = t->tag;
     if(!c->free) c->free = (rettrans == Success) || c->hint;
     free(t);

     client_attach(c);
     client_map(c);
     client_get_name(c);
     client_raise(c);
     client_focus(c);
     setwinstate(c->win, NormalState);
     arrange();

     return;
}

/** Move and Resize a client
 * \param c Client pointer
 * \param geo Coordinate info for the future size
 * of the client
 * \param r Bool for resize hint or not
*/
void
client_moveresize(Client *c, XRectangle geo, bool r)
{
     CHECK(c);

     int i;

     /* Resize hints {{{ */
     if(r)
     {
          /* minimum possible */
          if(geo.width < 1)
               geo.width = 1;
          if(geo.height < 1)
               geo.height = 1;

          /* base */
          geo.width -= c->basew;
          geo.height -= c->baseh;

          /* aspect */
          if(c->minay > 0 && c->maxay > 0
              && c->minax > 0 && c->maxax > 0)
          {
               if(geo.width * c->maxay > geo.height * c->maxax)
                    geo.width = geo.height * c->maxax / c->maxay;
               else if(geo.width * c->minay < geo.height * c->minax)
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

          /* Set the client screen */
          for(i = 0; i < screen_count(); ++i)
               if(geo.x >= sgeo[i].x
                  && geo.x < sgeo[i].x + sgeo[i].width
                  && geo.y >= sgeo[i].y - INFOBARH - TBARH
                  && geo.y < sgeo[i].y - INFOBARH - TBARH + sgeo[i].height + INFOBARH)
                    c->screen = i;

          frame_moveresize(c, geo);
          XMoveResizeWindow(dpy, c->win, BORDH, BORDH + TBARH, geo.width, geo.height);

          XSync(dpy, False);
     }

     return;
}

/** Get client size hints
 * \param c Client pointer
*/
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

/** Raise a client
 * \param c Client pointer
*/
void
client_raise(Client *c)
{
     if(!c || c->max || c->tile)
          return;

     XRaiseWindow(dpy, c->frame);

     return;
}

/** Raise the selected client
 * \param cmd uicb_t type unused
*/
void
uicb_client_raise(uicb_t cmd)
{
     client_raise(sel);

     return;
}

/** UnHide a client (for tag switching)
 * \param c Client pointer
*/
void
client_unhide(Client *c)
{
     CHECK(c->hide);

     client_map(c);
     c->hide = False;
     setwinstate(c->win, NormalState);

     return;
}

/** Unmanage a client
 * \param c Client pointer
*/
void
client_unmanage(Client *c)
{
     XGrabServer(dpy);
     XSetErrorHandler(errorhandlerdummy);
     XReparentWindow(dpy, c->win, root, c->geo.x, c->geo.y);

     if(sel == c)
          client_focus(NULL);

     client_detach(c);
     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     setwinstate(c->win, WithdrawnState);
     frame_delete(c);
     XSync(dpy, False);
     XUngrabServer(dpy);
     arrange();
     XFree(c->title);
     free(c);

     return;
}

/** Unmap a client
 * \param c Client pointer
*/
void
client_unmap(Client *c)
{
     CHECK(c);

     if(TBARH)
     {
          barwin_unmap_subwin(c->titlebar);
          barwin_unmap(c->titlebar);
     }
     XUnmapWindow(dpy, c->frame);
     XUnmapSubwindows(dpy, c->frame);
     c->unmapped = True;

     return;
}
