/*
*      client.c
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
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

#define client_swapsel(c)        \
     do                          \
     {                           \
          client_swap(sel, c);   \
          client_focus(c);       \
     } while(/* CONSTCOND */ 0)  \

#define CLIENT_ACTION_LIST(A, L)  \
void                              \
uicb_client_##A##_##L(uicb_t cmd) \
{                                 \
     Client *c;                   \
     (void)cmd;                   \
                                  \
     if((c = client_get_##L()))   \
          client_##A(c);          \
}

#define CLIENT_ACTION_DIR(A, D)                       \
void                                                  \
uicb_client_##A##_##D(uicb_t cmd)                     \
{                                                     \
     Client *c;                                       \
     (void)cmd;                                       \
                                                      \
     if((c = client_get_next_with_direction(sel, D))) \
          client_##A(c);                              \
}                                                     \

/* uicb_client_focus_List() */
CLIENT_ACTION_LIST(focus, next);
CLIENT_ACTION_LIST(focus, prev);

/* uicb_client_swap_List() */
CLIENT_ACTION_LIST(swapsel, next);
CLIENT_ACTION_LIST(swapsel, prev);

/* uicb_client_focus_Dir() */
CLIENT_ACTION_DIR(focus, Right);
CLIENT_ACTION_DIR(focus, Left);
CLIENT_ACTION_DIR(focus, Top);
CLIENT_ACTION_DIR(focus, Bottom);

/* uicb_client_swap_Dir() */
CLIENT_ACTION_DIR(swapsel, Right);
CLIENT_ACTION_DIR(swapsel, Left);
CLIENT_ACTION_DIR(swapsel, Top);
CLIENT_ACTION_DIR(swapsel, Bottom);

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

     ev.type              = ConfigureNotify;
     ev.event             = c->win;
     ev.window            = c->win;
     ev.x                 = c->geo.x;
     ev.y                 = c->geo.y;
     ev.width             = c->geo.width;
     ev.height            = c->geo.height;
     ev.above             = None;
     ev.border_width      = 0;
     ev.override_redirect = 0;

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

/** Get the next client
 *\return The next client or NULL
 */
Client*
client_get_next(void)
{
     Client *c = NULL;

     screen_get_sel();

     if(!sel || ishide(sel, selscreen))
          return NULL;

     for(c = sel->next; c && ishide(c, selscreen); c = c->next);

     if(!c && conf.client_round)
          for(c = clients; c && ishide(c, selscreen); c = c->next);

     return c;
}

/** Get the previous client
 *\return The previous client or NULL
 */
Client*
client_get_prev(void)
{
     Client *c = NULL, *d;

     screen_get_sel();

     if(!sel || ishide(sel, selscreen))
          return NULL;

     for(d = clients; d != sel; d = d->next)
          if(!ishide(d, selscreen))
               c = d;

     if(!c && conf.client_round)
          for(; d; d = d->next)
               if(!ishide(d, selscreen))
                    c = d;

     return c;
}

/** Get client left/right/top/bottom of selected client
  *\param bc Base client
  *\param pos Position (Left/Right/Top/Bottom
  *\return Client found
*/
Client*
client_get_next_with_direction(Client *bc, Position pos)
{
     Client *c;
     int x, y;
     char scanfac[4][2] =
     {
          { 1,  0 }, { -1, 0 }, /* Right, Left   */
          { 0, -1 }, {  0, 1 }  /* Top,   Bottom */
     };

     if(!bc)
          return NULL;

     /* Start place of pointer for faster scanning */
     x = bc->frame_geo.x + ((pos == Right)  ? bc->frame_geo.width  : 0);
     y = bc->frame_geo.y + ((pos == Bottom) ? bc->frame_geo.height : 0);
     y += ((LDIR(pos))  ? (bc->frame_geo.height >> 1) : 0);
     x += ((pos > Left) ? (bc->frame_geo.width  >> 1) : 0);

      /* Scan in right direction to next(p) physical client */
     for(; (c = client_gb_pos(bc, x, y)) == bc; x += scanfac[pos][0], y += scanfac[pos][1]);

     return c;
}

/** Set the client c above
  *\param c Client pointer
*/
static void
client_above(Client *c)
{
     Geo geo;

     memset(&geo, 0, sizeof(geo));

     if(c->flags & AboveFlag)
          return;

     c->flags |= AboveFlag;

     geo.height = spgeo[c->screen].height * 0.75;
     geo.width = spgeo[c->screen].width * 0.75;

     geo.y = spgeo[c->screen].y + ((spgeo[c->screen].height - geo.height) >> 1);
     geo.x = spgeo[c->screen].x + ((spgeo[c->screen].width - geo.width) >> 1);

     client_moveresize(c, geo, (tags[c->screen][c->tag].flags & ResizeHintFlag));
     client_raise(c);

     layout_func(c->screen, c->tag);

     return;
}

/** Set the focus to a client
 * \param c Client pointer
*/
void
client_focus(Client *c)
{
     bool prevwasmax = False;
     int prevscreen = 0;
     Client *cc;

     if(sel && sel != c)
     {
          if(sel->flags & AboveFlag)
               sel->flags &= ~AboveFlag;

          if(sel->flags & MaxFlag)
          {
               prevwasmax = True;
               prevscreen = sel->screen;
          }

          XChangeProperty(dpy, sel->frame, net_atom[net_wm_window_opacity], XA_CARDINAL,
                          32, PropModeReplace, (uchar *)&conf.opacity, 1);

          frame_update_color(sel, True);
          frame_update(sel);

          mouse_grabbuttons(sel, !conf.focus_pclick);
     }

     if((sel = c))
     {
          /* Set focusontag option */
          for(cc = clients; cc; cc = cc->next)
               if(cc->focusontag == (int)c->tag)
                    cc->focusontag = -1;

          c->focusontag = seltag[selscreen];

          XDeleteProperty(dpy, c->frame, net_atom[net_wm_window_opacity]);

          frame_update_color(c, False);
          frame_update(c);
          mouse_grabbuttons(c, True);

          if(conf.raisefocus || (c->flags & MaxFlag)
                    || (prevwasmax && prevscreen == c->screen))
          {
               client_raise(c);
               tags[c->screen][c->tag].flags |= IgnoreEnterFlag;
          }

          if((tags[sel->screen][sel->tag].flags & AboveFCFlag)
                    && !conf.focus_fmouse)
               client_above(sel);

          if(c->flags & UrgentFlag)
               client_urgent(c, False);

          XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);

          if(conf.bars.selbar)
               infobar_draw_selbar(&infobar[sel->screen]);
     }
     else
     {
          XSetInputFocus(dpy, ROOT, RevertToPointerRoot, CurrentTime);
          if(conf.bars.selbar)
               infobar_draw_selbar(&infobar[selscreen]);
     }

     return;
}

/** Set urgency flag of the client
 * \param c Client pointer
 * \param u bool
*/
void
client_urgent(Client *c, bool u)
{
     FLAGAPPLY(c->flags, u, UrgentFlag);
     FLAGAPPLY(tags[c->screen][c->tag].flags, u, TagUrgentFlag);

     infobar_draw_taglist(&infobar[c->screen]);
}

/* The following functions have the same point :
 * find a client member with a Window or values {{{
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

          if(!(TBARH - BORDH))
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

          for(c = clients; (c && c->resize[Right] != w) && (c && c->resize[Left] != w); c = c->next);

          return c;
     }

/** Get a client->button[button_num] with a window
 * \param w Window
 * \param n Pointer who return the button_num
 * \return The client
*/
     Client* client_gb_button(Window w, int *n)
     {
          Client *c;
          int i;

          if(!BUTTONWH || !(TBARH - BORDH))
               return NULL;

          for(c = clients; c; c = c->next)
               for(i = 0; i < conf.titlebar.nbutton; ++i)
                    if(c->button[i] == w)
                    {
                         *n = i;
                         return c;
                    }

          return NULL;
     }

/** Get a client with a position
 * \param x x value
 * \param y y value
 * \return The client
*/
     Client* client_gb_pos(Client *c, int x, int y)
     {
          Client *cc;

          if((x | y) < 0 || x > spgeo[c->screen].x + spgeo[c->screen].width
                    || y > spgeo[c->screen].y + spgeo[c->screen].height)
               return NULL;

          for(cc = clients; cc; cc = cc->next)
               if(cc != c && cc->screen == c->screen && cc->tag == c->tag
                         && (cc->flags & TileFlag))
                    if(cc->frame_geo.x < x && cc->frame_geo.x + cc->frame_geo.width > x
                              && cc->frame_geo.y < y && cc->frame_geo.y + cc->frame_geo.height > y)
                         return cc;

          return c;
     }

/* }}} */

/** Get a client name
 * \param c Client pointer
*/
void
client_get_name(Client *c)
{
     Atom rt;
     int rf;
     ulong ir, il;

     /* This one instead XFetchName for utf8 name support */
     if(XGetWindowProperty(dpy, c->win, net_atom[net_wm_name], 0, 4096,
                           False, net_atom[utf8_string], &rt, &rf, &ir, &il, (uchar**)&c->title) != Success)
          XGetWindowProperty(dpy, c->win, ATOM("WM_NAME"), 0, 4096,
                             False, net_atom[utf8_string], &rt, &rf, &ir, &il, (uchar**)&c->title);

     /* If there is no title... */
     if(!c->title)
     {
          XFetchName(dpy, c->win, &(c->title));

          if(!c->title)
               c->title = xstrdup("WMFS");
     }

     frame_update(c);

     if(conf.bars.selbar && c == sel)
          infobar_draw_selbar(&infobar[c->screen]);

     return;
}

/** Hide a client (for tag switching)
 * \param c Client pointer
*/
void
client_hide(Client *c)
{
     CHECK(!(c->flags & HideFlag));

     client_unmap(c);
     c->flags |= HideFlag;
     setwinstate(c->win, IconicState);

     return;
}

/** Check if the client 'c' is hide
 * \param c Client pointer
 * \return True if the client is hide; False if not
*/
bool
ishide(Client *c, int screen)
{
     if(c->screen == screen)
          if(c->tag == seltag[screen] || c->tag == MAXTAG + 1
                    || tags[screen][seltag[screen]].tagad & TagFlag(c->tag))
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
     int canbedel = 0;

     CHECK(c);

     if(XGetWMProtocols(dpy, c->win, &atom, &proto) && atom)
     {
          while(proto--)
               if(atom[proto] == ATOM("WM_DELETE_WINDOW"))
                    ++canbedel;
          XFree(atom);

          if(canbedel)
          {
               ev.type = ClientMessage;
               ev.xclient.window = c->win;
               ev.xclient.message_type = ATOM("WM_PROTOCOLS");
               ev.xclient.format = 32;
               ev.xclient.data.l[0] = ATOM("WM_DELETE_WINDOW");
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
     (void)cmd;
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

     if(c->flags & FSSFlag || c->flags & DockFlag)
          XMapWindow(dpy, c->win);
     else
     {
          XMapWindow(dpy, c->frame);
          XMapSubwindows(dpy, c->frame);
          if(TBARH - BORDH)
          {
               barwin_map(c->titlebar);
               barwin_map_subwin(c->titlebar);
          }
          XMapSubwindows(dpy, c->frame);
          c->flags &= ~UnmapFlag;
     }

     return;
}

/** Set the wanted tag or autofree/max of a client
 *\param c Client pointer
*/
static void
client_set_rules(Client *c)
{
     XClassHint xch;
     int i, f;
     Atom rf;
     ulong n, il;
     uchar *data = NULL;
     char wwrole[256] = { 0 };
     bool applied_tag_rule = False;
     bool applied_screen_rule = False;

     memset(&xch, 0, sizeof(xch));

     if(conf.ignore_next_client_rules)
     {
          conf.ignore_next_client_rules = False;
          return;
     }

     /* Get WM_CLASS */
     XGetClassHint(dpy, c->win, &xch);

     /* Get WM_WINDOW_ROLE */
     if(XGetWindowProperty(dpy, c->win, ATOM("WM_WINDOW_ROLE"), 0L, 0x7FFFFFFFL, False,
                    XA_STRING, &rf, &f, &n, &il, &data) == Success && data)
     {
          strncpy(wwrole, (char*)data, sizeof(wwrole));
          XFree(data);
     }

     /* Apply Rule if class || instance || role match */
     for(i = 0; i < conf.nrule; ++i)
     {
          if((xch.res_class && conf.rule[i].class && !strcmp(xch.res_class, conf.rule[i].class))
                    || (xch.res_name && conf.rule[i].instance && !strcmp(xch.res_name, conf.rule[i].instance)))
          {
               if((strlen(wwrole) && conf.rule[i].role && !strcmp(wwrole, conf.rule[i].role))
                         || (!strlen(wwrole) || !conf.rule[i].role))
               {
                    if(conf.rule[i].screen != -1)
                         c->screen = conf.rule[i].screen;

                    if(conf.rule[i].tag != -1)
                    {
                         c->tag = conf.rule[i].tag;
                         applied_tag_rule = True;
                    }

                    if(conf.rule[i].free)
                         c->flags |= FreeFlag;

                    if(conf.rule[i].ignoretags)
                         c->tag = MAXTAG + 1;

                    if(conf.rule[i].max)
                    {
                         c->flags |= MaxFlag;
                         client_maximize(c);
                    }

                    if(c->tag != (uint)seltag[selscreen])
                    {
                         tags[c->screen][c->tag].flags |= RequestUpdateFlag;
                         client_focus(NULL);
                    }

                    if(!conf.rule[i].ignoretags)
                         layout_func(c->screen, c->tag);

                    if(conf.rule[i].follow_client)
                         seltag[c->screen] = c->tag;
               }
          }
     }

     if(!applied_tag_rule && conf.client.default_open_tag > 0
          && conf.client.default_open_tag < (uint)conf.ntag[selscreen])
     {
          c->tag = conf.client.default_open_tag;

          client_focus_next(c);
          tags[c->screen][c->tag].flags |= RequestUpdateFlag;
     }

     if(!applied_screen_rule && conf.client.default_open_screen > -1
          && conf.client.default_open_screen < screen_count())
     {
          c->screen = conf.client.default_open_screen;

          client_focus_next(c);
          tags[c->screen][c->tag].flags |= RequestUpdateFlag;
     }

     return;
}

/** Manage a client with a window and his attributes
 * \param w Cient's futur Window
 * \param wa XWindowAttributes pointer, Window w attributes
 * \param ar Do arrange() or not (bool)
 * \return The managed client
*/
Client*
client_manage(Window w, XWindowAttributes *wa, bool ar)
{
     Client *c, *t = NULL;
     Window trans, dw;
     Status rettrans;
     XSetWindowAttributes at;
     int  mx, my, dint, x, y, d;
     uint duint;

     screen_get_sel();

     c = zmalloc(sizeof(Client));
     c->win = w;
     c->screen = selscreen;
     c->flags = 0;

     XQueryPointer(dpy, ROOT, &dw, &dw, &mx, &my, &dint, &dint, &duint);

     if(conf.client.place_at_mouse)
     {
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

          /* Check if the client is already in the selected
           * screen, else place the client in it */
          if(screen_get_with_geo(mx, my) != selscreen)
          {
               mx += spgeo[selscreen].x;
               my += spgeo[selscreen].y;
          }

          if(conf.client_auto_center)
          {
               Geo tmp;
               tmp = screen_get_geo(selscreen);
               mx = (tmp.width + mx - wa->width) >> 1;
               my = (tmp.height + my - wa->height) >> 1;
          }
     }

     c->ogeo.x = c->geo.x = mx;
     c->ogeo.y = c->geo.y = my;
     c->ogeo.width = c->geo.width = wa->width;
     c->ogeo.height = c->geo.height = wa->height;
     c->free_geo = c->pgeo = c->wrgeo = c->split_geo = c->geo;
     c->tag = seltag[c->screen];
     c->focusontag = -1;

     /* Set tilefact to 0 */
     tags[c->screen][c->tag].flags |= CleanFactFlag;
     cfactor_clean(c);

     at.event_mask = PropertyChangeMask;

     frame_create(c);
     client_size_hints(c);
     XChangeWindowAttributes(dpy, c->win, CWEventMask, &at);

     /* client win should _not_ have borders */
     XSetWindowBorderWidth(dpy, c->win, 0);
     mouse_grabbuttons(c, False);

     /* Transient for tag setting */
     if((rettrans = XGetTransientForHint(dpy, w, &trans) == Success))
          for(t = clients; t && t->win != trans; t = t->next);

     if(t)
     {
          c->tag = t->tag;
          c->screen = t->screen;
     }
     if(!(c->flags & FreeFlag)
          && (rettrans == Success || (c->flags & HintFlag)))
               c->flags |= FreeFlag;
     free(t);

     client_attach(c);
     client_set_rules(c);
     client_get_name(c);

     if(c->tag == (uint)seltag[selscreen])
     {
          client_raise(c);
          client_map(c);
          setwinstate(c->win, NormalState);
     }
     else
          client_hide(c);

     client_update_attributes(c);
     ewmh_get_client_list();
     ewmh_manage_window_type(c);

     split_set_current(c, NULL);

     if(ar)
          arrange(c->screen, True);

     if(!conf.client.set_new_win_master)
          layout_set_client_master(c);

     if(c->tag == (uint)seltag[selscreen])
          client_focus(c);

     if(conf.client.new_client_get_mouse)
     {
          /* Move pointer on client */
          XQueryPointer(dpy, ROOT, &dw, &dw, &x, &y, &d, &d, (uint *)&d);
          XWarpPointer(dpy, ROOT, ROOT, x, y, d, d,
                    c->geo.x + (c->geo.width  >> 1),
                    c->geo.y + (c->geo.height >> 1));
     }

     return c;
}

/** Set a geometry with the client size hints
 *\param geo Geometry pointer
 *\param c Client pointer
*/
void
client_geo_hints(Geo *geo, Client *c)
{
     /* minimum possible */
     if(geo->width < 1)
          geo->width = 1;
     if(geo->height < 1)
          geo->height = 1;

     /* base */
     geo->width -= c->basew;
     geo->height -= c->baseh;

     /* aspect */
     if((c->minay | c->maxay | c->minax | c->maxax) > 0)
     {
          if(geo->width * c->maxay > geo->height * c->maxax)
               geo->width = geo->height * c->maxax / c->maxay;
          else if(geo->width * c->minay < geo->height * c->minax)
               geo->height = geo->width * c->minay / c->minax;
     }

     /* incremental */
     if(c->incw)
          geo->width -= geo->width % c->incw;
     if(c->inch)
          geo->height -= geo->height % c->inch;

     /* base dimension */
     geo->width += c->basew;
     geo->height += c->baseh;

     if(c->minw > 0 && geo->width < c->minw)
          geo->width = c->minw;
     if(c->minh > 0 && geo->height < c->minh)
          geo->height = c->minh;
     if(c->maxw > 0 && geo->width > c->maxw)
          geo->width = c->maxw;
     if(c->maxh > 0 && geo->height > c->maxh)
          geo->height = c->maxh;

     return;
}

/** Move and Resize a client
 * \param c Client pointer
 * \param geo Coordinate info for the future size
 * of the client
 * \param r bool for resize hint or not
*/
void
client_moveresize(Client *c, Geo geo, bool r)
{
     int os, e;
     int rhx = 0;

     if(!c)
          return;

     os = c->screen;

     /* Apply padding and cfactor */
     if(c->flags & TileFlag)
     {
          geo = cfactor_geo(c->pgeo, c->tilefact, &e);

          if(conf.client.padding && (c->flags & FLayFlag))
          {
               geo.x += conf.client.padding;
               geo.y += conf.client.padding;
               geo.width -= (conf.client.padding << 1);
               geo.height -= (conf.client.padding << 1);

               c->flags &= ~FLayFlag;
          }
     }

     /* Set geo without resizehint applied */
     c->wrgeo = geo;

     /* Apply geometry hints */
     if(r)
     {
          client_geo_hints(&geo, c);

          /* To balance position of window in frame */
          rhx = (((c->wrgeo.width) - geo.width) >> 1);
      }

     c->geo = geo;

     /* Sett free_geo */
     if(c->flags & FreeFlag || !(c->flags & (TileFlag | LMaxFlag))
               || conf.keep_layout_geo)
          c->free_geo = c->geo;

     /* Check to set new screen if needed */
     if((c->screen = screen_get_with_geo(c->geo.x, c->geo.y)) != os
               && c->tag != MAXTAG + 1)
          c->tag = seltag[c->screen];

     frame_moveresize(c, c->wrgeo);

     XMoveResizeWindow(dpy, c->win, BORDH + rhx, TBARH, c->geo.width, c->geo.height);

     client_update_attributes(c);
     client_configure(c);

     return;
}

/** Maximize a client
 * \param c Client pointer
*/
void
client_maximize(Client *c)
{
     if(!c || c->flags & FSSFlag)
          return;

     cfactor_clean(c);
     c->screen = screen_get_with_geo(c->geo.x, c->geo.y);

     c->geo.x = sgeo[c->screen].x;
     c->geo.y = sgeo[c->screen].y ;
     c->geo.width  = sgeo[c->screen].width  - (BORDH << 1);
     c->geo.height = sgeo[c->screen].height - BORDH;

     client_moveresize(c, (c->pgeo = c->geo), (tags[c->screen][c->tag].flags & ResizeHintFlag));

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
          c->minw = (size.min_width ? size.min_width : 1);
          c->minh = (size.min_height ? size.min_height: 1);
     }
     else if(size.flags & PBaseSize)
     {
          c->minw = (size.base_width ? size.base_width : 1);
          c->minh = (size.base_height ? size.base_height : 1);
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

     if(c->maxw && c->minw && c->maxh && c->minh
        && c->maxw == c->minw && c->maxh == c->minh)
          c->flags |= HintFlag;

     return;
}

/** Swap two clients
 *\param 1 c1 First client
 *\param 2 c2 Second client
*/
void
client_swap(Client *c1, Client *c2)
{
     int tmp;

     /* Check if no one of these clients are free */
     CHECK(!(c1->flags & FreeFlag));
     CHECK(!(c2->flags & FreeFlag));

     if(c1 == c2)
          return;

     if(c1->tag != c2->tag)
     {
          tmp = c1->tag;
          c1->tag = c2->tag;
          c2->tag = tmp;
     }

     /* Swap only the windows */
     swap_ptr((void**)&c1->win, (void**)&c2->win);

     /* Re-adapt the windows position with its new frame */
     XReparentWindow(dpy, c1->win, c1->frame, BORDH, TBARH);
     XReparentWindow(dpy, c2->win, c2->frame, BORDH, TBARH);

     /* Re-set size hints properties */
     client_size_hints(c1);
     client_size_hints(c2);

     /* Resize the windows */
     client_moveresize(c1, c1->geo, (tags[c1->screen][c1->tag].flags & ResizeHintFlag));
     client_moveresize(c2, c2->geo, (tags[c2->screen][c2->tag].flags & ResizeHintFlag));

     /* Get the new client name */
     client_get_name(c1);
     client_get_name(c2);

     return;
}

/** Update client attributes (_WMFS_TAG _WMFS_SCREEN)
 *\param c Client pointer
*/
void
client_update_attributes(Client *c)
{
     /* For reload use */
     XChangeProperty(dpy, c->win, ATOM("_WMFS_TAG"), XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&(c->tag), 1);

     XChangeProperty(dpy, c->win, ATOM("_WMFS_SCREEN"), XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&(c->screen), 1);

     XChangeProperty(dpy, c->win, ATOM("_WMFS_FLAGS"), XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&(c->flags), 1);

     return;
}

/** Raise a client
 * \param c Client pointer
*/
void
client_raise(Client *c)
{
     if((!c || ((c->flags & TileFlag) && !(c->flags & (AboveFlag | MaxFlag))))
               && !conf.client_tile_raise)
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
     (void)cmd;
     CHECK(sel);

     client_raise(sel);

     return;
}

/** UnHide a client (for tag switching)
 * \param c Client pointer
*/
void
client_unhide(Client *c)
{
     CHECK(c->flags & HideFlag);

     client_map(c);
     c->flags &= ~HideFlag;
     setwinstate(c->win, NormalState);

     return;
}

/** Select next or previous client to don't lose focus
  * \param c Client pointer
  */
void
client_focus_next(Client *c)
{
     Client *c_next = NULL;

     for(c_next = clients;
         c_next && c_next != c->prev
              && c_next->tag != c->tag
              && c_next->screen != c->screen;
         c_next = c_next->next);

     if(c_next && c_next->tag == (uint)seltag[selscreen]
        && c_next->screen == selscreen)
          client_focus(c_next);

     return;
}

/** Unmanage a client
 * \param c Client pointer
*/
void
client_unmanage(Client *c)
{
     bool b = False;
     int i;

     XGrabServer(dpy);
     XSetErrorHandler(errorhandlerdummy);
     XReparentWindow(dpy, c->win, ROOT, c->geo.x, c->geo.y);

     if(sel == c)
          client_focus(NULL);

     if(c->flags & UrgentFlag)
          client_urgent(c, False);

     client_detach(c);
     XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
     setwinstate(c->win, WithdrawnState);
     frame_delete(c);
     XSync(dpy, False);
     XUngrabServer(dpy);
     ewmh_get_client_list();

     if(c->flags & TileFlag)
     {
          tags[c->screen][c->tag].flags |= CleanFactFlag;
          split_set_current(NULL, c);
     }

     if(c->tag == MAXTAG + 1)
     {
          for(i = 0; i < conf.ntag[c->screen]; i++)
               tags[c->screen][i].flags |= RequestUpdateFlag;
          layout_func(c->screen, seltag[c->screen]);
     }
     else
     {
          /* Arrange */
          for(i = 0; i < screen_count() && !b; ++i)
               if(c->tag == (uint)seltag[i])
               {
                    b = True;
                    break;
               }
               else if(tags[i][seltag[i]].tagad & TagFlag(c->tag))
               {
                    layout_func(i, seltag[i]);
                    break;
               }

          if(b)
               layout_func(c->screen, c->tag);
          else
          {
               tags[c->screen][c->tag].flags |= RequestUpdateFlag;
               infobar_draw(&infobar[c->screen]);
          }
     }

     client_focus_next(c);

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

     if(c->flags & FSSFlag)
          XUnmapWindow(dpy, c->win);
     else
     {
          if(TBARH - BORDH)
          {
               barwin_unmap_subwin(c->titlebar);
               barwin_unmap(c->titlebar);
          }

          XUnmapWindow(dpy, c->frame);
          XUnmapSubwindows(dpy, c->frame);
          c->flags |= UnmapFlag;
     }

     return;
}

/** Set the client screen
 *\param c Client pointer
 *\param s Screen id
 */
static void
client_set_screen(Client *c, int s)
{
     int os;
     Geo geo;

     if(!c || s < 0 || s > screen_count() - 1 || s == c->screen)
          return;

     /* Save old client screen/geo to arrange */
     geo = c->geo;
     os = c->screen;

     c->screen = s;
     c->tag = seltag[s];

     /* Arrange */
     if(tags[s][seltag[s]].layout.func == freelayout
        || tags[s][seltag[s]].layout.func == maxlayout
        || tags[os][seltag[os]].layout.func == freelayout
        || tags[os][seltag[os]].layout.func == maxlayout)
     {
          geo.x = ((sgeo[s].x + sgeo[s].width - c->geo.width) >> 1);
          geo.y = ((sgeo[s].y + sgeo[s].height - c->geo.width) >> 1);
          client_moveresize(c, geo, False);
     }

     arrange(s, True);
     arrange(os, True);

     if(!(c->flags & TileFlag))
     {
          client_focus(c);
          client_raise(c);
     }

     return;
}

/** Change client of screen to next screen
 * \param cmd uicb_t type unused
*/
void
uicb_client_screen_next(uicb_t cmd)
{
     (void)cmd;
     CHECK(sel);

     client_set_screen(sel, (sel->screen + 1 > screen_count() - 1) ? 0 : sel->screen + 1);

     return;
}

/** Change client of screen to prev screen
 * \param cmd uicb_t type unused
*/
void
uicb_client_screen_prev(uicb_t cmd)
{
     (void)cmd;
     CHECK(sel);

     client_set_screen(sel, (sel->screen - 1 < 0) ? screen_count() - 1 : sel->screen - 1);

     return;
}

/** Change client of screen to n
 * \param cmd uicb_t type screen
*/
void
uicb_client_screen_set(uicb_t cmd)
{
     (void)cmd;
     CHECK(sel);

     client_set_screen(sel, atoi(cmd));

     return;
}

/** Move a client
 *\param cmd uicb_t type
 */
void
uicb_client_move(uicb_t cmd)
{
     Geo geo;
     int xi = 0, yi = 0;

     if(!sel || sel->flags & (TileFlag | MaxFlag | LMaxFlag | FSSFlag))
          return;

     geo = sel->geo;

     if(sscanf(cmd, "%d %d", &xi, &yi))
     {
          geo.x += xi;
          geo.y += yi;

          client_moveresize(sel, geo, False);
     }

     return;
}

/** Resize a client
 *\param cmd uicb_t type
 */
void
uicb_client_resize(uicb_t cmd)
{
     Geo geo;
     int wi = 0, hi = 0;

     if(!sel || sel->flags & (TileFlag | MaxFlag | LMaxFlag | FSSFlag))
          return;

     geo = sel->geo;

     if(sscanf(cmd, "%d %d", &wi, &hi))
     {
          geo.width += ((geo.width + wi > sel->minw && geo.width + wi < 65536) ? wi : 0);
          geo.height += ((geo.height + hi > sel->minh && geo.height + hi < 65536) ? hi : 0);

          client_moveresize(sel, geo, False);
     }

     return;
}

/** Ignore next client rules
  *\param cmd uicb_t type
  */
void
uicb_ignore_next_client_rules(uicb_t cmd)
{
     (void)cmd;
     conf.ignore_next_client_rules = !conf.ignore_next_client_rules;

     return;
}

/** Select client
 *\param cmd uicb_t type clientlist index
 */
static void
uicb_client_select(uicb_t cmd)
{
     int i;
     Window w;
     int d, x, y;


     for(i = 0; i < MAXCLIST && clist_index[i].client; ++i)
          if(!strcmp(cmd, clist_index[i].key))
          {
               if(clist_index[i].client->screen != selscreen)
                    screen_set_sel(clist_index[i].client->screen);

               if(clist_index[i].client->tag != (uint)seltag[clist_index[i].client->screen])
                    tag_set(clist_index[i].client->tag);

               client_focus(clist_index[i].client);
               client_raise(clist_index[i].client);

               /* Move pointer on client */
               XQueryPointer(dpy, ROOT, &w, &w, &x, &y, &d, &d, (uint *)&d);
               XWarpPointer(dpy, ROOT, ROOT, x, y, d, d,
                         clist_index[i].client->geo.x + (clist_index[i].client->geo.width >> 1),
                         clist_index[i].client->geo.y + (clist_index[i].client->geo.height >> 1));
          }

     return;
}

/** Show clientlist menu
 *\param cmd uicb_t type
 */
void
uicb_clientlist(uicb_t cmd)
{
     int i, d, u, x, y;
     int n = 0;
     bool all = False;
     Window w;
     Client *c = NULL;

     screen_get_sel();

     if(cmd && !strcmp(cmd, "all"))
          all = True;

     for(c = clients; c; c = c->next)
          if(!ishide(c, selscreen) || all)
               ++n;

     if(n > 0)
     {
          if(clientlist.nitem)
               menu_clear(&clientlist);

          menu_init(&clientlist, "clientlist", n,
                    /* Colors */
                    conf.colors.tagselbg,
                    conf.colors.tagselfg,
                    conf.colors.bar,
                    conf.colors.text);

          clientlist.align = MA_Left;

          for(i = 0, c = clients; c; c = c->next)
               if(!ishide(c, selscreen) || all)
               {
                    sprintf(clist_index[i].key, "%d", i);
                    clist_index[i].client = c;

                    menu_new_item(&clientlist.item[i], c->title,
                                  uicb_client_select, clist_index[i].key);

                    if(c == sel)
                         clientlist.item[i].check = name_to_func("check_clist", func_list);

                    i++;
               }

          clist_index[i].client = NULL;

          XQueryPointer(dpy, ROOT, &w, &w, &x, &y, &d, &d, (uint *)&u);
          menu_draw(clientlist, x, y);
     }

     return;
}

/** Check clientlist menu fake function
 * \param cmd uicb_t type unused
*/
bool
uicb_checkclist(uicb_t cmd)
{
     (void)cmd;
     return True;
}

/** Set selected client on all tag (ignore tags
  *\para cmd uicb_t type unused
*/
void
uicb_client_ignore_tag(uicb_t cmd)
{
     (void)cmd;
     CHECK(sel);

     screen_get_sel();

     sel->tag = ((sel->tag == MAXTAG + 1) ? seltag[selscreen] : MAXTAG + 1);

     arrange(sel->screen, True);

     return;
}

/** Set current client as master
  *\para cmd uicb_t type unused
*/
void
uicb_client_set_master(uicb_t cmd)
{
     Client *c;
     (void)cmd;

     /* get the first client */
     screen_get_sel();
     if(!sel || ishide(sel, selscreen))
          return;

     for(c = clients; c && ishide(c, selscreen); c = c->next);

     if (c && c != sel)
     {
          client_swap(c, sel);
          client_focus(c);
     }
     return;
}

