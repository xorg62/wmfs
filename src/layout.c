/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "layout.h"
#include "config.h"
#include "client.h"
#include "util.h"

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

     client_moveresize(c, &c->geo);

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

     SLIST_FOREACH(cc, &c->tag->clients, tnext)
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
               SLIST_FOREACH(cc, &c->tag->clients, tnext)
                    if(GEO_PARENTROW(g, cc->geo, RPOS(p))
                              && GEO_CHECK_ROW(cc->geo, ghost->geo, p))
                    {
                         layout_split_arrange_size(&ghost->geo, cc, p);
                         b = true;
                    }
          }
     }
}

/* Integrate a client in split layout: split sc and fill c in new geo */
void
layout_split_integrate(struct client *c, struct client *sc)
{
     struct geo g;

     /* No sc */
     if(!sc || sc == c || sc->tag != c->tag)
     {
          /*
           * Not even a first client in list, then
           * maximize the lonely client
           */
          if(!(sc = SLIST_FIRST(&c->tag->clients)))
          {
               client_maximize(c);
               return;
          }
     }

     g = layout_split(sc, (sc->geo.h < sc->geo.w));
     client_moveresize(c, &g);
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
static inline void
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
layout_rotate(struct tag *t, bool left)
{
     struct client *c;
     struct geo g;
     float f1 = (float)t->screen->ugeo.w / (float)t->screen->ugeo.h;
     float f2 = 1 / f1;
     void (*pos)(struct geo*, struct geo*, struct geo*) =
          (left ? _pos_rotate_left : _pos_rotate_right);

     SLIST_FOREACH(c, &t->clients, tnext)
     {
          pos(&g, &t->screen->ugeo, &c->geo);

          g.x *= f1;
          g.y *= f2;
          g.w = c->geo.h * f1;
          g.h = c->geo.w * f2;

          client_moveresize(c, &g);
     }

     /* Rotate sometimes do not set back perfect size.. */
     SLIST_FOREACH(c, &t->clients, tnext)
          layout_fix_hole(c);
}

void
uicb_layout_rotate_left(Uicb cmd)
{
     (void)cmd;
     layout_rotate(W->screen->seltag, true);
}

void
uicb_layout_rotate_right(Uicb cmd)
{
     (void)cmd;
     layout_rotate(W->screen->seltag, false);
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

     SLIST_FOREACH(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.x = W->screen->ugeo.w - (c->geo.x + c->geo.w);
          client_moveresize(c, &c->geo);
     }
}

void
uicb_layout_hmirror(Uicb cmd)
{
     (void)cmd;
     struct client *c;

     SLIST_FOREACH(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.y = W->screen->ugeo.h - (c->geo.y + c->geo.h);
          client_moveresize(c, &c->geo);
     }
}
