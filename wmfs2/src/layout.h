/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef LAYOUT_H
#define LAYOUT_H

#include "wmfs.h"
#include "config.h"
#include "client.h"

/* Check lateral direction (if p is Right or Left) */
#define LDIR(P) (P < Top)

/* Reverse position */
#define RPOS(P) (P & 1 ? P - 1 : P + 1)

/* geo comparaison */
#define GEO_CHECK2(g1, g2, p) (LDIR(p) ? (g1.h == g2.h) : (g1.w == g2.w))
#define GEO_CHECK_ROW(g1, g2, p)                          \
     (LDIR(p)                                             \
      ? (g1.y >= g2.y && (g1.y + g1.h) <= (g2.y + g2.h))  \
      : (g1.x >= g2.x && (g1.x + g1.w) <= (g2.x + g2.w)))
#define GEO_PARENTROW(g1, g2, p)                                     \
     (LDIR(p)                                                        \
      ? (p == Left ? (g1.x == g2.x) : (g1.x + g1.w == g2.x + g2.w))  \
      : (p == Top  ? (g1.y == g2.y) : (g1.y + g1.h == g2.y + g2.h)))


/* Debug */
#define DGEO(G) printf(": %d %d %d %d\n", G.x, G.y, G.w, G.h)


void layout_split_integrate(struct client *c, struct client *sc);
void layout_split_arrange_closed(struct client *ghost);

static inline void
layout_split_arrange_size(struct geo g, struct client *c, Position p)
{
     if(LDIR(p))
     {
          c->geo.w += g.w + (THEME_DEFAULT->client_border_width << 1);

          if(p == Right)
               c->geo.x = g.x;
     }
     else
     {
          c->geo.h += g.h + (THEME_DEFAULT->client_border_width << 1);

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
     int bord = THEME_DEFAULT->client_border_width;
     int i = 0, s = 0, cs = (LDIR(p) ? g->geo.h : g->geo.w );


     SLIST_FOREACH(cc, &c->tag->clients, tnext)
          if(GEO_PARENTROW(cgeo, cc->geo, RPOS(p))
                    && GEO_CHECK_ROW(cc->geo, g->geo, p))
          {
               s += (LDIR(p)
                         ? cc->geo.h + bord
                         : cc->geo.w + bord) + (i > 1 ? bord : 0);;

               if(s == cs)
                    return true;
               if(s > cs)
                    return false;

               ++i;
          }

     return false;
}

#endif /* LAYOUT_H */

