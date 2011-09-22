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

     client_moveresize(c, c->geo);

     return geo;
}

static inline void
layout_split_arrange_size(struct geo g, struct client *c, Position p)
{
     if(LDIR(p))
     {
          c->geo.w += g.w;

          if(p == Right)
               c->geo.x = g.x;
     }
     else
     {
          c->geo.h += g.h;

          if(p == Bottom)
               c->geo.y = g.y;
     }

     client_moveresize(c, c->geo);
}

static inline bool
layout_split_check_row_dir(struct client *c, struct client *g, Position p)
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
 *     .--.  ~   ~
 *    /xx  \   ~   ~
 *  ~~\O _ (____     ~
 *  __.|    .--'-==~   ~
 * '---\    '.      ~  ,  ~
 *      '.    '-.___.-'/   ~
 *        '-.__     _.'  ~
 *             `````   ~
 */
void
layout_split_arrange_closed(struct client *ghost)
{
     struct client *c, *cc;
     struct geo g;
     bool b = false;
     Position p;


     /* Search for single parent for easy resize
      * Example case:
      *  ___________               ___________
      * |     |  B  | ->       -> |     |     |
      * |  A  |_____| -> Close -> |  A  |  B  |
      * |     |  C  | ->   C   -> |     |v v v|
      * |_____|_____| ->       -> |_____|_____|
      */
     for(p = Right; p < Center; ++p)  /* Check every direction */
     {
          if((c = client_next_with_pos(ghost, p)))
               if(GEO_CHECK2(ghost->geo, c->geo, p))
               {
                    layout_split_arrange_size(ghost->geo, c, p);
                    return;
               }
     }

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
                         layout_split_arrange_size(ghost->geo, cc, p);
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
     client_moveresize(c, g);
}

void
layout_rotate(struct tag *t, bool left)
{
     struct client *c;
     struct geo g, ug = t->screen->ugeo;

     SLIST_FOREACH(c, &t->clients, tnext)
     {
          /*
          g = c->geo;
          c->geo.x = (g.y * ug.h) / ug.w;
          c->geo.y = (g.x * ug.w) / ug.h;

          c->geo.w = (g.h * ug.w) / ug.h;
          c->geo.h = (g.w * ug.h) / ug.w;

          client_moveresize(c, c->geo);
          */
     }
}

void
uicb_layout_rotate(Uicb cmd)
{
     layout_rotate(W->screen->seltag, false);
}

/*
 * Really simple functions, don't need static no-uicb backend
 * so we avoid the use of if(vertical) .. else
 */
void
uicb_layout_vmirror(Uicb cmd)
{
     struct client *c;

     SLIST_FOREACH(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.x = W->screen->ugeo.w - (c->geo.x + c->geo.w);
          client_moveresize(c, c->geo);
     }
}

void
uicb_layout_hmirror(Uicb cmd)
{
     struct client *c;

     SLIST_FOREACH(c, &W->screen->seltag->clients, tnext)
     {
          c->geo.y = W->screen->ugeo.h - (c->geo.y + c->geo.h);
          client_moveresize(c, c->geo);
     }
}
