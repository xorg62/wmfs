/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h>

#include "layout.h"
#include "config.h"
#include "client.h"
#include "draw.h"
#include "event.h"
#include "util.h"

void
layout_save_set(struct tag *t)
{
     struct client *c;
     struct layout_set *l;
     struct geo_list *g, *gp;
     int n = 1;

     l = xcalloc(1, sizeof(struct layout_set));
     SLIST_INIT(&l->geos);

     FOREACH_NFCLIENT(c, &t->clients, tnext)
     {
          g = xcalloc(1, sizeof(struct geo_list));
          g->geo = c->geo;

          if(!SLIST_FIRST(&l->geos))
               SLIST_INSERT_HEAD(&l->geos, g, next);
          else
               SLIST_INSERT_AFTER(gp, g, next);

          ++n;
          gp = g;
     }

     l->n = n;

     TAILQ_INSERT_TAIL(&t->sets, l, next);
}

static void
layout_apply_set(struct tag *t, struct layout_set *l)
{
     struct geo_list *g;
     struct client *c;
     int nc = 1;

     FOREACH_NFCLIENT(c, &t->clients, tnext)
          ++nc;

     /* TODO: Adapt different client number case */
     if(l->n != nc)
          return;

     for(g = SLIST_FIRST(&l->geos), c = SLIST_FIRST(&t->clients);
         c; c = SLIST_NEXT(c, tnext))
     {
          if(g)
          {
               client_moveresize(c, &g->geo);
               g = SLIST_NEXT(g, next);
          }
          /* TODO
           * Not enough geos in the set;
           * then integrate remains of client
           *

          else
               layout_split_integrate(c, SLIST_FIRST(&t->clients));
           */
     }

     /* TODO
      * Not enough clients for geos in set;
      * arrange clients with not set geo.
      *
     if((g = SLIST_NEXT(g, next)))
          for(cc.tag = t; g; g = SLIST_NEXT(g, next))
          {
               cc.geo = g->geo;
               layout_split_arrange_closed(&cc);
          }
     */

     /* Re-insert set in historic */
     layout_save_set(t);
}

void
layout_free_set(struct tag *t)
{
     struct layout_set *l;

     TAILQ_FOREACH(l, &t->sets, next)
     {
          TAILQ_REMOVE(&t->sets, l, next);
          FREE_LIST(geo_list, l->geos);
          free(l);
     }
}

#define _REV_BORDER()                              \
     SLIST_FOREACH(g, &l->geos, next) {            \
          cd.geo = g->geo;                         \
          draw_reversed_rect(W->root, &cd, false); \
     }
static void
_historic_set(struct tag *t, bool prev)
{
     struct keybind *k;
     struct layout_set *l;
     struct geo_list *g;
     struct client cd = { .screen = t->screen, .theme = THEME_DEFAULT };
     bool b = true;
     XEvent ev;
     KeySym keysym;

     if(TAILQ_EMPTY(&t->sets))
          return;

     l = TAILQ_LAST(&t->sets, ssub);

     if(prev)
          l = TAILQ_PREV(l, ssub, next);

     if(!l)
          return;

     /* TODO
     if(option_simple_manual_resize)
     {
          layout_set_apply(l);
          return;
     */

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
                         if(k->func == uicb_layout_prev_set)
                         {
                              if(!(l = TAILQ_PREV(l, ssub, next)))
                                   l = TAILQ_LAST(&t->sets, ssub);
                         }
                         else if(k->func == uicb_layout_next_set)
                         {
                              if(!(l = TAILQ_NEXT(l, next)))
                                   l = TAILQ_FIRST(&t->sets);
                         }
                         else
                         {
                              k->func(k->cmd);
                              keysym = XK_Escape;
                         }
                    }

               if(!l)
                    l = TAILQ_LAST(&t->sets, ssub);

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

     if(b)
          layout_apply_set(t, l);

     XUngrabServer(W->dpy);
     XUngrabKeyboard(W->dpy, CurrentTime);
}

void
uicb_layout_prev_set(Uicb cmd)
{
     (void)cmd;

     _historic_set(W->screen->seltag, true);
}

void
uicb_layout_next_set(Uicb cmd)
{
     (void)cmd;

     _historic_set(W->screen->seltag, false);
}

static struct geo
layout_split(struct client *c, bool vertical)
{
     struct geo og, geo;

     geo = og = c->geo;

     if(vertical)
     {
          c->geo.w >>= 1;
          geo.x = c->geo.x + c->geo.w;
          geo.w >>= 1;

          /* Remainder */
          geo.w += (og.x + og.w) - (geo.x + geo.w);
     }
     else
     {
          c->geo.h >>= 1;
          geo.y = c->geo.y + c->geo.h;
          geo.h >>= 1;

          /* Remainder */
          geo.h += (og.y + og.h) - (geo.y + geo.h);
     }

     return geo;
}

static inline void
layout_split_arrange_size(struct geo *g, struct client *c, enum position p)
{
     if(LDIR(p))
     {
          c->geo.w += g->w;

          if(p == Right)
               c->geo.x = g->x;
     }
     else
     {
          c->geo.h += g->h;

          if(p == Bottom)
               c->geo.y = g->y;
     }

     client_moveresize(c, &c->geo);
}

static inline bool
layout_split_check_row_dir(struct client *c, struct client *g, enum position p)
{
     struct geo cgeo = c->geo;
     struct client *cc;
     int s = 0, cs = (LDIR(p) ? g->geo.h : g->geo.w);

     FOREACH_NFCLIENT(cc, &c->tag->clients, tnext)
          if(GEO_PARENTROW(cgeo, cc->geo, RPOS(p))
             && GEO_CHECK_ROW(cc->geo, g->geo, p))
          {
               s += (LDIR(p) ? cc->geo.h : cc->geo.w);

               if(s == cs)
                    return true;
               if(s > cs)
                    return false;
          }

     return false;
}

/* Use ghost client properties to fix holes in tile
 *
 *     ~   .--.  ~   ~
 *_____ ~ /xx  \   ~   ~
 *  |>>| ~\O _ (____     ~
 *  |  |__.|    .--'-==~   ~
 *  |>>'---\    '.      ~  ,  ~
 *__|__|    '.    '-.___.-'/   ~
 *            '-.__     _.'  ~
 *                 `````   ~
 */
#define _ARRANGE_SINGLE_PARENT(p)                                 \
     do {                                                         \
          if((c = client_next_with_pos(ghost, p)))                \
               if(GEO_CHECK2(ghost->geo, c->geo, p))              \
               {                                                  \
                    layout_split_arrange_size(&ghost->geo, c, p); \
                    layout_save_set(ghost->tag);                  \
                    return;                                       \
               }                                                  \
     } while(/* CONSTCOND */ 0);
void
layout_split_arrange_closed(struct client *ghost)
{
     struct client *c, *cc;
     struct geo g;
     bool b = false;
     enum position p;

     if(!(ghost->flags & CLIENT_TILED))
          return;

     /* Search for single parent for easy resize
      * Example case:
      *  ___________               ___________
      * |     |  B  | ->       -> |     |     |
      * |  A  |_____| -> Close -> |  A  |  B  |
      * |     |  C  | ->   C   -> |     |v v v|
      * |_____|_____| ->       -> |_____|_____|
      */
     _ARRANGE_SINGLE_PARENT(Right);
     _ARRANGE_SINGLE_PARENT(Left);
     _ARRANGE_SINGLE_PARENT(Top);
     _ARRANGE_SINGLE_PARENT(Bottom);

     /* Check row parents for full resize
      * Example case:
      *  ___________               ___________
      * |     |  B  | ->       -> | <<  B     |
      * |  A  |_____| -> Close -> |___________|
      * |     |  C  | ->   A   -> | <<  C     |
      * |_____|_____| ->       -> |___________|
      */
     for(p = Right; p < Center && !b; ++p)
     {
          if((c = client_next_with_pos(ghost, p))
                    && layout_split_check_row_dir(c, ghost, p))
          {
               g = c->geo;
               FOREACH_NFCLIENT(cc, &c->tag->clients, tnext)
                    if(GEO_PARENTROW(g, cc->geo, RPOS(p))
                       && GEO_CHECK_ROW(cc->geo, ghost->geo, p))
                    {
                         layout_split_arrange_size(&ghost->geo, cc, p);
                         b = true;
                    }
          }
     }

     layout_save_set(ghost->tag);
}

/* Integrate a client in split layout: split sc and fill c in new geo */
void
layout_split_integrate(struct client *c, struct client *sc)
{
     struct geo g;
     bool f = false;

     /* No sc or not compatible sc */
     if(!sc || sc == c || sc->tag != c->tag
        || sc->flags & CLIENT_FREE)
     {
          /*
           * check for the first tiled client or
           * maximize the lonely client
           */
          FOREACH_NFCLIENT(sc, &c->tag->clients, tnext)
               if(sc != c && !(sc->flags & CLIENT_TABBED))
               {
                    f = true;
                    break;
               }

          /* Ok there is no client to integrate in */
          if(!f)
          {
               client_maximize(c);
               c->flags |= CLIENT_TILED;
               return;
          }
     }

     /* Tab Next Opened Client */
     if(W->flags & WMFS_TABNOC)
     {
          W->flags ^= WMFS_TABNOC;
          _client_tab(c, sc);
          return;
     }

     c->flags |= CLIENT_TILED;

     g = layout_split(sc, (sc->geo.h < sc->geo.w));

     client_moveresize(c, &g);
     client_moveresize(sc, &sc->geo);

     client_fac_hint(c);
     client_fac_hint(sc);

     layout_save_set(c->tag);
}

/* Arrange inter-clients holes:
 *  ___________      ___________
 * |     ||    | -> |      |    |
 * |  A  || B  | -> |  A  >| B  |
 * |     ||    | -> |     >|    |
 * |_____||____| -> |______|____|
 *        ^ void
 *
 * and client-screen edge holes
 *  ___________      ___________
 * |     |    || -> |     |     |
 * |  A  |  B || -> |  A  |  B >|
 * |     |    || -> |     |    >|
 * |_____|----'| -> |_____|__v__|
 *          ^^^ void
 */
void
layout_fix_hole(struct client *c)
{
     struct client *cr = client_next_with_pos(c, Right);
     struct client *cb = client_next_with_pos(c, Bottom);

     c->geo.w += (cr ? cr->geo.x : c->screen->ugeo.w) - (c->geo.x + c->geo.w);
     c->geo.h += (cb ? cb->geo.y : c->screen->ugeo.h) - (c->geo.y + c->geo.h);

     client_moveresize(c, &c->geo);
}

/* Layout rotation: Rotate 90° all client to right or left.
 * Avoid if(left) condition in layout_rotate loop; use func ptr
 *
 * Right rotation
 *  ____________        ____________
 * |    |   B   |  ->  |  |   A     |
 * |  A |_______|  ->  |__|_________|
 * |____| C | D |  ->  |_____|   B  |
 * |____|___|___|  ->  |_____|______|
 *
 * Left rotation
 *  ____________        ____________
 * |    |   B   |  ->  |   B  |_____|
 * |  A |_______|  ->  |______|_____|
 * |____| C | D |  ->  |     A   |  |
 * |____|___|___|  ->  |_________|__|
 *
 */

static inline void
_pos_rotate_left(struct geo *g, struct geo *ug, struct geo *og)
{
     g->x = (ug->h - (og->y + og->h));
     g->y = og->x;
}

static inline void
_pos_rotate_right(struct geo *g, struct geo *ug, struct geo *og)
{
     g->x = og->y;
     g->y = (ug->w - (og->x + og->w));
}

static void
layout_rotate(struct tag *t, void (*pfunc)(struct geo*, struct geo*, struct geo*))
{
     struct client *c;
     struct geo g, *ug = &t->screen->ugeo;
     float f1 = (float)t->screen->ugeo.w / (float)t->screen->ugeo.h;
     float f2 = 1 / f1;

     FOREACH_NFCLIENT(c, &t->clients, tnext)
     {
          pfunc(&g, ug, &c->geo);

          g.x *= f1;
          g.y *= f2;

          g.w = c->geo.h * f1;
          g.h = c->geo.w * f2;

          client_moveresize(c, &g);
     }

     /* Rotate sometimes do not set back perfect size.. */
     FOREACH_NFCLIENT(c, &t->clients, tnext)
          layout_fix_hole(c);

     layout_save_set(t);
}

void
uicb_layout_rotate_left(Uicb cmd)
{
     (void)cmd;
     layout_rotate(W->screen->seltag, _pos_rotate_left);
}

void
uicb_layout_rotate_right(Uicb cmd)
{
     (void)cmd;
     layout_rotate(W->screen->seltag, _pos_rotate_right);
}

/*
 * Really simple functions, don't need static no-uicb backend
 * so we avoid the use of if(vertical) .. else
 *
 * Vertical mirror
 *  ____________        ____________
 * |    |   B   |  ->  |   B   |    |
 * |  A |_______|  ->  |_______| A  |
 * |    | C | D |  ->  | D | C |    |
 * |____|___|___|  ->  |___|___|____|
 *
 * Horizontal mirror
 *  ____________        ____________
 * |    |   B   |  ->  |    | C | D |
 * |  A |_______|  ->  |  A |___|___|
 * |    | C | D |  ->  |    |   B   |
 * |____|___|___|  ->  |____|_______|
 */
void
uicb_layout_vmirror(Uicb cmd)
{
     (void)cmd;
     struct client *c;

     FOREACH_NFCLIENT(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.x = W->screen->ugeo.w - (c->geo.x + c->geo.w);
          client_moveresize(c, &c->geo);
     }

     layout_save_set(W->screen->seltag);
}

void
uicb_layout_hmirror(Uicb cmd)
{
     (void)cmd;
     struct client *c;

     FOREACH_NFCLIENT(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.y = W->screen->ugeo.h - (c->geo.y + c->geo.h);
          client_moveresize(c, &c->geo);
     }

     layout_save_set(W->screen->seltag);
}

#define LAYOUT_INTEGRATE_DIR(D)                                         \
     void uicb_layout_integrate_##D(Uicb cmd)                           \
     {                                                                  \
          (void)cmd;                                                    \
          if(W->client)                                                 \
               layout_integrate(W->client, D);                          \
     }
static void
layout_integrate(struct client *c, enum position p)
{
     struct client *n;
     struct client ghost = *c;

     if(!(c->flags & CLIENT_TILED))
          return;

     if((n = client_next_with_pos(c, p))
        && (n->flags & CLIENT_TILED))
     {
          layout_split_integrate(c, n);
          layout_split_arrange_closed(&ghost);
     }
}

LAYOUT_INTEGRATE_DIR(Left);
LAYOUT_INTEGRATE_DIR(Right);
LAYOUT_INTEGRATE_DIR(Top);
LAYOUT_INTEGRATE_DIR(Bottom);

void
layout_client(struct client *c)
{
     if(c->flags & CLIENT_IGNORE_LAYOUT)
     {
          c->flags ^= CLIENT_IGNORE_LAYOUT;
          return;
     }

     if(c->flags & CLIENT_FREE)
     {
          layout_split_arrange_closed(c);
          c->flags ^= CLIENT_TILED;
          client_moveresize(c, &c->geo);
          XRaiseWindow(W->dpy, c->frame);
     }
     else if(!(c->flags & CLIENT_TABBED))
          layout_split_integrate(c, c->tag->sel);
}
