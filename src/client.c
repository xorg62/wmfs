/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h>

#include "client.h"
#include "config.h"
#include "event.h"
#include "barwin.h"
#include "barwin.h"
#include "draw.h"
#include "screen.h"

#define CLIENT_MOUSE_MOD Mod1Mask

#define CLIENT_RESIZE_DIR(D)                          \
void uicb_client_resize_##D(Uicb cmd)                 \
{                                                     \
     if(W->client)                                    \
          client_fac_resize(W->client, D, ATOI(cmd)); \
}

#define CLIENT_ACTION_DIR(A, D)                                \
void uicb_client_##A##_##D(Uicb cmd)                           \
{                                                              \
     (void)cmd;                                                \
     struct client *c;                                         \
     if(W->client && (c = client_next_with_pos(W->client, D))) \
          client_##A(c);                                       \
}

#define CLIENT_ACTION_IDIR(A, D)     \
void uicb_client_##A##_##D(Uicb cmd) \
{                                    \
     (void)cmd;                      \
     if(W->client)                   \
          client_##A(W->client, D);  \
}

#define CLIENT_ACTION_LIST(A, L)                  \
void uicb_client_##A##_##L(Uicb cmd)              \
{                                                 \
     (void)cmd;                                   \
     struct client *c;                            \
     if(W->client && (c = client_##L(W->client))) \
          client_##A(c);                          \
}

/* uicb_client_resize_dir() */
CLIENT_RESIZE_DIR(Right)
CLIENT_RESIZE_DIR(Left)
CLIENT_RESIZE_DIR(Top)
CLIENT_RESIZE_DIR(Bottom)

/* uicb_client_focus_dir() */
CLIENT_ACTION_DIR(focus, Right)
CLIENT_ACTION_DIR(focus, Left)
CLIENT_ACTION_DIR(focus, Top)
CLIENT_ACTION_DIR(focus, Bottom)

/* uicb_client_tab_dir() */
#define client_tab(c) _client_tab(c, W->client)
CLIENT_ACTION_DIR(tab, Right)
CLIENT_ACTION_DIR(tab, Left)
CLIENT_ACTION_DIR(tab, Top)
CLIENT_ACTION_DIR(tab, Bottom)

/* uicb_client_swap_dir() */
CLIENT_ACTION_IDIR(swap, Right)
CLIENT_ACTION_IDIR(swap, Left)
CLIENT_ACTION_IDIR(swap, Top)
CLIENT_ACTION_IDIR(swap, Bottom)

/* uicb_client_focus_next/prev() */
CLIENT_ACTION_LIST(focus, next)
CLIENT_ACTION_LIST(focus, prev)

/* uicb_client_swapsel_next/prev() */
#define client_swapsel(c) client_swap2(W->client, c)
CLIENT_ACTION_LIST(swapsel, next)
CLIENT_ACTION_LIST(swapsel, prev)

/** Send a ConfigureRequest event to the struct client
 * \param c struct client pointer
*/
inline void
client_configure(struct client *c)
{
     XConfigureEvent ev =
     {
          .type              = ConfigureNotify,
          .event             = c->win,
          .window            = c->win,
          .x                 = c->rgeo.x + c->border,
          .y                 = c->rgeo.y + c->tbarw,
          .width             = c->wgeo.w,
          .height            = c->wgeo.h,
          .above             = None,
          .border_width      = 0,
          .override_redirect = 0
     };

     XSendEvent(W->dpy, c->win, False, StructureNotifyMask, (XEvent *)&ev);
     XSync(W->dpy, False);
}

struct client*
client_gb_win(Window w)
{
     struct client *c = SLIST_FIRST(&W->h.client);

     while(c && c->win != w)
          c = SLIST_NEXT(c, next);

     return c;
}

struct client*
client_gb_frame(Window w)
{
     struct client *c = SLIST_FIRST(&W->h.client);

     while(c && c->frame != w)
          c = SLIST_NEXT(c, next);

     return c;
}

struct client*
client_gb_pos(struct tag *t, int x, int y)
{
     struct client *c = SLIST_FIRST(&t->clients);

     while(c)
     {
          if(INAREA(x, y, c->geo))
               return c;

          c = SLIST_NEXT(c, tnext);
     }

     return NULL;
}

/*
 * Get client left/right/top/bottom of selected client
 */
struct client*
client_next_with_pos(struct client *bc, enum position p)
{
     struct client *c;
     static const char scanfac[PositionLast] = { +10, -10, 0, 0 };
     enum position ip = Bottom - p;
     int x = bc->geo.x + ((p == Right)  ? bc->geo.w : 0);
     int y = bc->geo.y + ((p == Bottom) ? bc->geo.h : 0);

     if(p > Left)
          x += bc->geo.w >> 1;
     if(LDIR(p))
          y += bc->geo.h >> 1;

     /* Scan in right direction to next(p) physical client */
     while((c = client_gb_pos(bc->tag, x, y)) == bc)
     {
          x += scanfac[p];
          y += scanfac[ip];
     }

     return c;
}

void
client_swap2(struct client *c1, struct client *c2)
{
     struct tag *t;

     /* Conflict / errors */
     if(c1 == c2 || !c1 || !c2)
          return;

     /* are swapped geos compatible? */
     if(client_winsize(c1, &c2->geo)
        || client_winsize(c2, &c1->geo))
          return;

     if(c1->screen != c2->screen)
          swap_ptr((void**)&c1->screen, (void**)&c2->screen);

     if(c1->tag != c2->tag)
     {
          t = c1->tag;
          tag_client(c2->tag, c1);
          tag_client(t, c2);
     }

     c1->tgeo = c2->geo;
     c2->tgeo = c1->geo;

     c1->flags |= CLIENT_IGNORE_ENTER;
     c2->flags |= CLIENT_IGNORE_ENTER;

     client_moveresize(c1, &c1->tgeo);
     client_moveresize(c2, &c2->tgeo);
}

static inline struct client*
_swap_get(struct client *c, enum position p)
{
     struct client *ret = client_next_with_pos(c, p);

     if(!ret)
          return c;

     return ret;
}

#define _REV_SBORDER() \
          draw_reversed_rect(W->root, &c2->geo);
void
client_swap(struct client *c, enum position p)
{
     struct keybind *k;
     struct client *c2;
     bool b = true;
     XEvent ev;
     KeySym keysym;

     c2 = _swap_get(c, p);

     /* TODO
     if(option_simple_manual_resize)
     {
        _swap(c, c2);
        return;
     }
     */

     XGrabKeyboard(W->dpy, W->root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
     _REV_SBORDER();

     do
     {
          XMaskEvent(W->dpy, KeyPressMask, &ev);

          if(ev.type == KeyPress)
          {
               XKeyPressedEvent *ke = &ev.xkey;
               keysym = XKeycodeToKeysym(W->dpy, (KeyCode)ke->keycode, 0);

               _REV_SBORDER();

               SLIST_FOREACH(k, &W->h.keybind, next)
                    if(k->keysym == keysym && KEYPRESS_MASK(k->mod) == KEYPRESS_MASK(ke->state)
                              && k->func)
                    {
                         if(k->func == uicb_client_swap_Right)
                              c2 = _swap_get(c2, Right);
                         else if(k->func == uicb_client_swap_Left)
                              c2 = _swap_get(c2, Left);
                         else if(k->func == uicb_client_swap_Top)
                              c2 = _swap_get(c2, Top);
                         else if(k->func == uicb_client_swap_Bottom)
                              c2 = _swap_get(c2, Bottom);
                         else
                         {
                              k->func(k->cmd);
                              keysym = XK_Escape;
                         }
                    }

               _REV_SBORDER();

               /* Gtfo of this loop */
               if(keysym == XK_Return)
                    break;
               else if(keysym == XK_Escape)
               {
                    b = false;
                    break;
               }

               XSync(W->dpy, False);
          }
          XNextEvent(W->dpy, &ev);

     } while(ev.type != KeyPress);

     _REV_SBORDER();

     if(b)
         client_swap2(c, c2);

     XUngrabKeyboard(W->dpy, CurrentTime);
}

static void
client_grabbuttons(struct client *c, bool focused)
{
     XUngrabButton(W->dpy, AnyButton, AnyModifier, c->win);

     if(focused)
     {
          int i = 0;

          while(i++ != Button5)
          {
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | LockMask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | W->numlockmask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
               XGrabButton(W->dpy, i, CLIENT_MOUSE_MOD | LockMask | W->numlockmask, c->win, False,
                         ButtonMask, GrabModeAsync, GrabModeSync, None, None);
          }

          return;
     }

     XGrabButton(W->dpy, AnyButton, AnyModifier, c->win, False,
               ButtonMask, GrabModeAsync, GrabModeSync, None, None);
}

void
client_frame_update(struct client *c, struct colpair *cp)
{
     XSetWindowBackground(W->dpy, c->frame, cp->bg);
     XClearWindow(W->dpy, c->frame);

     if(c->titlebar && c->title)
     {
          struct client *cc;
          int f, n = 1, xt, w = draw_textw(c->theme, c->title);

          c->titlebar->fg = cp->fg;
          c->titlebar->bg = cp->bg;

          SLIST_FOREACH(cc, &c->tag->clients, tnext)
               if(cc->tbgeo == &c->geo)
                    ++n;

          barwin_reparent(c->titlebar, c->frame);
          barwin_move(c->titlebar, 0, 0);
          barwin_resize(c->titlebar, (f = (c->geo.w / n)), c->tbarw);
          barwin_refresh_color(c->titlebar);

          if((xt = ((f >> 1) - (w >> 1) - PAD)) < 0)
               xt = 0;

          draw_text(c->titlebar->dr, c->theme, xt,
                    TEXTY(c->theme, c->tbarw), cp->fg, c->title);
          barwin_refresh(c->titlebar);

          /* Tabbing case, multiple titlebar in frame */
          if(n > 1)
          {
               int x = f;

               SLIST_FOREACH(cc, &c->tag->clients, tnext)
                    if(cc->tbgeo == &c->geo && cc->titlebar)
                    {
                         cc->titlebar->bg = c->ncol.bg;

                         w = draw_textw(c->theme, cc->title);
                         if((xt = ((f >> 1) - (w >> 1) - PAD)) < 0)
                              xt = 0;

                         barwin_map(cc->titlebar);
                         barwin_reparent(cc->titlebar, c->frame);
                         barwin_move(cc->titlebar, x, 0);
                         barwin_resize(cc->titlebar, f, c->tbarw);
                         barwin_refresh_color(cc->titlebar);

                         draw_text(cc->titlebar->dr, c->theme, xt,
                                   TEXTY(c->theme, c->tbarw), c->ncol.fg, cc->title);

                         barwin_refresh(cc->titlebar);

                         x += f + 1;
                    }
          }
     }
}

void
client_focus(struct client *c)
{
     /* Unfocus selected */
     if(W->client && W->client != c)
     {
          client_grabbuttons(W->client, false);
          client_frame_update(W->client, &W->client->ncol);
     }

     /* Focus c */
     if((W->client = c))
     {
          c->tag->sel = c;
          client_grabbuttons(c, true);

          if(c->flags & CLIENT_TABBED)
          {
               struct geo ocg = c->geo;
               struct client *cc, *tc;

               SLIST_FOREACH(cc, &c->tag->clients, tnext)
                    if(c->tbgeo == &cc->geo)
                    {
                         c->flags  &= ~CLIENT_TABBED;
                         cc->flags |=  CLIENT_TABBED;

                         client_map(c);
                         client_unmap(cc);

                         client_moveresize(c, c->tbgeo);
                         client_moveresize(cc, &ocg);

                         cc->tbgeo = &c->geo;
                         c->tbgeo  = NULL;

                         SLIST_FOREACH(tc, &c->tag->clients, tnext)
                              if(tc->flags & CLIENT_TABBED && tc->tbgeo == &cc->geo)
                                   tc->tbgeo = &c->geo;

                         break;
                    }
          }

          client_frame_update(c, &c->scol);
          XSetInputFocus(W->dpy, c->win, RevertToPointerRoot, CurrentTime);
     }
     else
     {
          W->client = W->screen->seltag->sel = NULL;
          XSetInputFocus(W->dpy, W->root, RevertToPointerRoot, CurrentTime);
     }
}

/** Get a client name
 * \param c struct client pointer
*/
void
client_get_name(struct client *c)
{
     Atom rt;
     int rf;
     unsigned long ir, il;

     /* This one instead XFetchName for utf8 name support */
     if(XGetWindowProperty(W->dpy, c->win, ATOM("_NET_WM_NAME"), 0, 4096,
                    False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, (unsigned char**)&c->title) != Success)
          XGetWindowProperty(W->dpy, c->win, ATOM("WM_NAME"), 0, 4096,
                             False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, (unsigned char**)&c->title);

     /* Still no title... */
     if(!c->title)
          XFetchName(W->dpy, c->win, &(c->title));

     client_frame_update(c, CCOL(c));
}

/** Close a client
 * \param c struct client pointer
*/
void
client_close(struct client *c)
{
     int proto;
     XEvent ev;
     Atom *atom = NULL;

     /* Event will call client_remove */
     if(XGetWMProtocols(W->dpy, c->win, &atom, &proto) && atom)
     {
          while(proto--)
               if(atom[proto] == ATOM("WM_DELETE_WINDOW"))
               {
                    ev.type = ClientMessage;
                    ev.xclient.window = c->win;
                    ev.xclient.message_type = ATOM("WM_PROTOCOLS");
                    ev.xclient.format = 32;
                    ev.xclient.data.l[0] = ATOM("WM_DELETE_WINDOW");
                    ev.xclient.data.l[1] = CurrentTime;

                    XSendEvent(W->dpy, c->win, False, NoEventMask, &ev);
                    XFree(atom);

                    return;
               }
     }

     XKillClient(W->dpy, c->win);
}

void
uicb_client_close(Uicb cmd)
{
     (void)cmd;

     if(W->client)
          client_close(W->client);
}

static void
client_get_sizeh(struct client *c)
{
     long msize;
     XSizeHints size;

     memset(c->sizeh, 0, SHLAST);

     if(!XGetWMNormalHints(W->dpy, c->win, &size, &msize) || !size.flags)
          size.flags = PSize;

     /* base */
     if(size.flags & PBaseSize)
     {
          c->sizeh[BASEW] = size.base_width;
          c->sizeh[BASEH] = size.base_height;
     }
     else if(size.flags & PMinSize)
     {
          c->sizeh[BASEW] = size.min_width;
          c->sizeh[BASEH] = size.min_height;
     }

     /* inc */
     if(size.flags & PResizeInc)
     {
          c->sizeh[INCW] = size.width_inc;
          c->sizeh[INCH] = size.height_inc;
     }

     /* max */
     if(size.flags & PMaxSize)
     {
          c->sizeh[MAXW] = size.max_width;
          c->sizeh[MAXH] = size.max_height;
     }

     /* min */
     if(size.flags & PMinSize)
     {
          c->sizeh[MINW] = (size.min_width ? size.min_width : 1);
          c->sizeh[MINH] = (size.min_height ? size.min_height: 1);
     }
     else if(size.flags & PBaseSize)
     {
          c->sizeh[MINW] = (size.base_width ? size.base_width : 1);
          c->sizeh[MINH] = (size.base_height ? size.base_height : 1);
     }

     /* aspect */
     if(size.flags & PAspect)
     {
          c->sizeh[MINAX] = size.min_aspect.x;
          c->sizeh[MAXAX] = size.max_aspect.x;
          c->sizeh[MINAY] = size.min_aspect.y;
          c->sizeh[MAXAY] = size.max_aspect.y;
     }

     if(c->sizeh[MAXW] && c->sizeh[MINW] && c->sizeh[MAXH] && c->sizeh[MINH]
        && c->sizeh[MAXW] == c->sizeh[MINW] && c->sizeh[MAXH] == c->sizeh[MINH])
          c->flags |= CLIENT_HINT_FLAG;
}

static void
client_frame_new(struct client *c)
{
     XSetWindowAttributes at =
     {
          .background_pixel  = c->ncol.bg,
          .override_redirect = true,
          .background_pixmap = ParentRelative,
          .event_mask        = BARWIN_MASK | BARWIN_ENTERMASK
     };

     c->frame = XCreateWindow(W->dpy, W->root,
                              c->geo.x, c->geo.y,
                              c->geo.w, c->geo.h,
                              0, CopyFromParent,
                              InputOutput,
                              CopyFromParent,
                              (CWOverrideRedirect | CWBackPixmap
                               | CWBackPixel | CWEventMask), &at);

     if(c->tbarw > c->border)
          c->titlebar = barwin_new(c->frame, 0, 0, 1, c->tbarw,
                                   c->ncol.fg, c->ncol.bg, true);

     XReparentWindow(W->dpy, c->win, c->frame, c->border, c->tbarw);
}

#define RINSTANCE 0x01
#define RCLASS    0x02
#define RROLE     0x04
#define RNAME     0x08
static void
client_apply_rule(struct client *c)
{
     struct rule *r;
     char *wmname = NULL;
     char *role = NULL;
     int f;
     unsigned char *data = NULL;
     unsigned long n, il;
     Flags flags = 0;
     Atom rf;
     XClassHint xch;

     XGetClassHint(W->dpy, c->win, &xch);

     /* Get WM_WINDOW_ROLE */
     if(XGetWindowProperty(W->dpy, c->win, ATOM("WM_WINDOW_ROLE"), 0L, 0x7FFFFFFFL, false,
                           XA_STRING, &rf, &f, &n, &il, &data)
               == Success && data)
     {
          role = xstrdup((char*)data);
          XFree(data);
     }

     /* Get _NET_WM_NAME */
     if(XGetWindowProperty(W->dpy, c->win, ATOM("_NET_WM_NAME"), 0, 0x77777777, false,
                           ATOM("UTF8_STRING"), &rf, &f, &n, &il, &data)
               == Success && data)
     {
          wmname = xstrdup((char*)data);
          XFree(data);
     }

     SLIST_FOREACH(r, &W->h.rule, next)
     {
          FLAGAPPLY(flags, (xch.res_name && r->instance && !strcmp(xch.res_name, r->instance)),      RINSTANCE);
          FLAGAPPLY(flags, (xch.res_class && r->class && !strcmp(xch.res_class, r->class)),          RCLASS);
          FLAGAPPLY(flags, ((role && r->role && !strcmp(role, r->role)) || !role || !r->role),       RROLE);
          FLAGAPPLY(flags, ((wmname && r->name && !strcmp(wmname, r->name)) || !wmname || !r->name), RNAME);

          if(flags & RINSTANCE || flags & RCLASS)
          {
               if(flags & RROLE || flags & RNAME)
               {
                    c->screen = screen_gb_id(r->screen);
                    c->tag    = tag_gb_id(c->screen, r->tag);
                    c->theme  = r->theme;

                    if(r->flags & RULE_FREE)
                    { /* TODO */ }

                    if(r->flags & RULE_MAX)
                    { /* TODO */ }

                    if(r->flags & RULE_IGNORE_TAG)
                    { /* TODO */ }

                    c->flags  = r->flags | CLIENT_RULED;
               }
          }

          flags = 0;
     }

     if(role)
          free(role);

     if(wmname)
          free(wmname);
}

struct client*
client_new(Window w, XWindowAttributes *wa, bool scan)
{
     struct client *c = xcalloc(1, sizeof(struct client));

     /* C attributes */
     c->win    = w;
     c->flags  = 0;
     c->screen = W->screen;
     c->theme  = THEME_DEFAULT;
     c->tag    = NULL;
     c->tabmaster = NULL;

     /* struct geometry */
     c->geo.x = wa->x;
     c->geo.y = wa->y;
     c->geo.w = wa->width;
     c->geo.h = wa->height;
     c->tgeo = c->wgeo = c->rgeo = c->geo;
     c->tbgeo = NULL;

     client_apply_rule(c);

     /*
      * Conf option set per client, for possibility
      * to config only one client
      */
     c->border = c->theme->client_border_width;
     if(!(c->tbarw = c->theme->client_titlebar_width))
          c->tbarw = c->border;

     c->ncol = c->theme->client_n;
     c->scol = c->theme->client_s;

     client_frame_new(c);

     /* Set tag */
     client_get_sizeh(c);

     if(!scan)
          tag_client((c->flags & CLIENT_RULED ? c->tag : c->screen->seltag), c);

     /* Map */
     if(c->tag == c->screen->seltag)
          client_map(c);

     /* X window attributes */
     XSelectInput(W->dpy, w, EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask);
     XSetWindowBorderWidth(W->dpy, w, 0);
     client_grabbuttons(c, false);

     /* Attach */
     SLIST_INSERT_HEAD(&W->h.client, c, next);

     ewmh_set_wm_state(w, NormalState);

     if(!scan)
     {
          client_get_name(c);
          client_focus(c);
          client_configure(c);
     }

     return c;
}

void
client_update_props(struct client *c, Flags f)
{
     if(f & CPROP_LOC)
     {
          XChangeProperty(W->dpy, c->win, ATOM("_WMFS_TAG"), XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)&(c->tag->id), 1);

          XChangeProperty(W->dpy, c->win, ATOM("_WMFS_SCREEN"), XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)&(c->screen->id), 1);
     }

     if(f & CPROP_FLAG)
          XChangeProperty(W->dpy, c->win, ATOM("_WMFS_FLAGS"), XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)&(c->flags), 1);

     if(f & CPROP_GEO)
     {
          long g[4] = { (long)c->geo.x, (long)c->geo.y, (long)c->geo.w, (long)c->geo.h };

          XChangeProperty(W->dpy, c->win, ATOM("_WMFS_GEO"), XA_CARDINAL, 32,
                          PropModeReplace, (unsigned char*)g, 4);

     }
}

static void
client_geo_hints(struct geo *g, int *s)
{
     /* base */
     g->w -= s[BASEW];
     g->h -= s[BASEH];

     /* aspect */
     if((s[MINAY] | s[MAXAY] | s[MINAX] | s[MAXAX]) > 0)
     {
          if(g->w * s[MAXAY] > g->h * s[MAXAX])
               g->w = g->h * s[MAXAX] / s[MAXAY];
          else if(g->w * s[MINAY] < g->h * s[MINAX])
               g->h = g->w * s[MINAY] / s[MINAX];
     }

     /* incremental */
     if(s[INCW])
          g->w -= g->w % s[INCW];
     if(s[INCH])
          g->h -= g->h % s[INCH];

     /* base dimension */
     g->w += s[BASEW];
     g->h += s[BASEH];

     if(s[MINW] > 0 && g->w < s[MINW])
          g->w = s[MINW];
     if(s[MINH] > 0 && g->h < s[MINH])
          g->h = s[MINH];
     if(s[MAXW] > 0 && g->w > s[MAXW])
          g->w = s[MAXW];
     if(s[MAXH] > 0 && g->h > s[MAXH])
          g->h = s[MAXH];
}

bool
client_winsize(struct client *c, struct geo *g)
{
     int ow, oh;
     struct geo og = c->wgeo;

     /* Window geo */
     c->wgeo.x = c->border;
     c->wgeo.y = c->tbarw;
     c->wgeo.h = oh = g->h - (c->border + c->tbarw);
     c->wgeo.w = ow = g->w - (c->border << 1);

     client_geo_hints(&c->wgeo, (int*)c->sizeh);

     /* Check possible problem for tile integration */
     if(ow < c->sizeh[MINW] || oh < c->sizeh[MINH])
          if(ow + oh < og.w + og.h)
          {
               c->wgeo = og;
               return true;
          }

     /* Balance position with new size */
     c->wgeo.x += (ow - c->wgeo.w) >> 1;

     c->flags |= CLIENT_DID_WINSIZE;

     return false;
}

void
client_moveresize(struct client *c, struct geo *g)
{
     struct geo og = c->geo;

     if(c->flags & CLIENT_TABBED)
          return;

     c->ttgeo = c->tgeo = c->rgeo = c->geo = *g;

     if(!(c->flags & CLIENT_DID_WINSIZE))
          if(client_winsize(c, g))
          {
               /* TODO
                * Window required size not compatible
                * with frame window size in tile mode
                */
          }

     /* Real geo regarding full root size */
     if(!(c->flags & CLIENT_TABSLAVE))
     {
          c->rgeo.x += c->screen->ugeo.x;
          c->rgeo.y += c->screen->ugeo.y;
     }

     XMoveResizeWindow(W->dpy, c->frame,
                       c->rgeo.x, c->rgeo.y,
                       c->rgeo.w, c->rgeo.h);

     XMoveResizeWindow(W->dpy, c->win,
                       c->wgeo.x, c->wgeo.y,
                       c->wgeo.w, c->wgeo.h);

     c->flags &= ~CLIENT_DID_WINSIZE;

     client_frame_update(c, CCOL(c));
     client_update_props(c, CPROP_GEO);
     client_configure(c);
}

void
client_maximize(struct client *c)
{
     c->geo.x = c->geo.y = 0;
     c->geo.w = c->screen->ugeo.w;
     c->geo.h = c->screen->ugeo.h;

     client_moveresize(c, &c->geo);

     layout_save_set(c->tag);
}

/*
 * Client factor resize: allow clients to be resized in
 * manual tile layout.
 */
static inline void
_fac_apply(struct client *c, enum position p, int fac)
{
     switch(p)
     {
          case Top:
               c->tgeo.y -= fac;
          case Bottom:
               c->tgeo.h += fac;
               break;
          case Left:
               c->tgeo.x -= fac;
          default:
          case Right:
               c->tgeo.w += fac;
               break;
     }

     c->flags |= (CLIENT_IGNORE_ENTER | CLIENT_FAC_APPLIED);
}

static inline void
_fac_arrange_row(struct client *c, enum position p, int fac)
{
     struct geo g = c->tgeo;
     struct client *cc;

     /* Travel clients to search row parents and apply fac */
     SLIST_FOREACH(cc, &c->tag->clients, tnext)
          if(GEO_PARENTROW(g, cc->tgeo, p))
               _fac_apply(cc, p, fac);
}

void
_fac_resize(struct client *c, enum position p, int fac)
{
     struct client *cc, *gc = client_next_with_pos(c, p);
     enum position rp = RPOS(p);

     if(!gc || gc->screen != c->screen)
          return;

     SLIST_FOREACH(cc, &c->tag->clients, tnext)
          cc->ttgeo = cc->tgeo;

     if(GEO_CHECK2(c->geo, gc->geo, p))
     {
          _fac_apply(c, p, fac);
          _fac_apply(gc, rp, -fac);
     }
     else
     {
          _fac_arrange_row(c, p, fac);
          _fac_arrange_row(gc, rp, -fac);
     }

     /*
      * Check if every clients are compatible with
      * future globals geo. Problem here is that we must *not*
      * resize client because of possible error with next
      * clients in linked list.
      */
     SLIST_FOREACH(gc, &c->tag->clients, tnext)
          if(gc->flags & CLIENT_FAC_APPLIED
             && client_winsize(gc, &gc->tgeo))
          {
               /*
                * Reverse back the flag and the window geo
                * in previous affected clients
                */
               SLIST_FOREACH(cc, &c->tag->clients, tnext)
               {
                    cc->tgeo = cc->ttgeo;
                    cc->flags &= ~CLIENT_DID_WINSIZE;
               }

               return;
          }
}

void
client_apply_tgeo(struct tag *t)
{
     struct client *c;

     SLIST_FOREACH(c, &t->clients, tnext)
     {
          client_moveresize(c, &c->tgeo);
          c->flags &= ~CLIENT_FAC_APPLIED;
     }
}

#define _REV_BORDER()                                        \
     do {                                                    \
          SLIST_FOREACH(gc, &c->tag->clients, tnext)         \
               draw_reversed_rect(W->root, &gc->tgeo);       \
      /* draw_reversed_cross(W->root, &c->tag->sel->tgeo);*/ \
     } while(/* CONSTCOND */ 0);
void
client_fac_resize(struct client *c, enum position p, int fac)
{
     struct keybind *k;
     struct client *gc;
     bool b = true;
     XEvent ev;
     KeySym keysym;

     /* Do it once before */
     _fac_resize(c, p, fac);

     /* TODO
     if(option_simple_manual_resize)
          return;
     */

     XGrabServer(W->dpy);
     XGrabKeyboard(W->dpy, W->root, True, GrabModeAsync, GrabModeAsync, CurrentTime);

     _REV_BORDER();

     do
     {
          XMaskEvent(W->dpy, KeyPressMask, &ev);

          if(ev.type == KeyPress)
          {
               XKeyPressedEvent *ke = &ev.xkey;
               keysym = XKeycodeToKeysym(W->dpy, (KeyCode)ke->keycode, 0);

               _REV_BORDER();

               SLIST_FOREACH(k, &W->h.keybind, next)
                    if(k->keysym == keysym && KEYPRESS_MASK(k->mod) == KEYPRESS_MASK(ke->state)
                              && k->func)
                    {
                         if(k->func == uicb_client_resize_Right)
                              _fac_resize(c, Right, ATOI(k->cmd));
                         else if(k->func == uicb_client_resize_Left)
                              _fac_resize(c, Left, ATOI(k->cmd));
                         else if(k->func == uicb_client_resize_Top)
                              _fac_resize(c, Top, ATOI(k->cmd));
                         else if(k->func == uicb_client_resize_Bottom)
                              _fac_resize(c, Bottom, ATOI(k->cmd));
                         else
                         {
                              k->func(k->cmd);
                              keysym = XK_Escape;
                         }
                    }

               _REV_BORDER();

               /* Gtfo of this loop */
               if(keysym == XK_Return)
                    break;
               else if(keysym == XK_Escape)
               {
                    b = false;
                    break;
               }

               XSync(W->dpy, False);
          }
          XNextEvent(W->dpy, &ev);

     } while(ev.type != KeyPress);

     _REV_BORDER();

     /* Success, resize clients */
     if(b)
     {
          client_apply_tgeo(c->tag);
          layout_save_set(c->tag);
     }
     /* Aborted with escape, Set back original geos */
     else
     {
          SLIST_FOREACH(gc, &c->tag->clients, tnext)
          {
               gc->tgeo = gc->geo;
               gc->flags &= ~CLIENT_DID_WINSIZE;
          }
     }

     XUngrabServer(W->dpy);
     XUngrabKeyboard(W->dpy, CurrentTime);
}

inline void
client_fac_hint(struct client *c)
{
     int w = c->sizeh[MINW] + c->border + c->border;
     int h = c->sizeh[MINH] + c->tbarw + c->border;

     if(c->geo.w < w)
          _fac_resize(c, Right, (w - c->geo.w));
     if(c->tgeo.w < w)
          _fac_resize(c, Left, (w - c->tgeo.w));

     if(c->geo.h < h)
          _fac_resize(c, Top, (h - c->geo.h));
     if(c->tgeo.h < h)
          _fac_resize(c, Bottom, (h - c->tgeo.h));

     client_apply_tgeo(c->tag);
}

void
client_remove(struct client *c)
{
     XGrabServer(W->dpy);
     XSetErrorHandler(wmfs_error_handler_dummy);
     XReparentWindow(W->dpy, c->win, W->root, c->rgeo.x, c->rgeo.y);
     WIN_STATE(c->win, Map);
     XDestroyWindow(W->dpy, c->frame);

     if(c->titlebar)
          barwin_remove(c->titlebar);

     /* Remove from global client list */
     SLIST_REMOVE(&W->h.client, c, client, next);

     tag_client(NULL, c);

     ewmh_set_wm_state(c->win, WithdrawnState);

     XUngrabServer(W->dpy);
     XSync(W->dpy, False);
     XSetErrorHandler(wmfs_error_handler);

     free(c);
}

void
client_free(void)
{
     FREE_LIST(client, W->h.client);
}
