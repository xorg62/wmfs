/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "wmfs.h"
#include "layout.h"

void client_configure(struct client *c);
struct client *client_gb_win(Window w);
struct client *client_gb_pos(struct tag *t, int x, int y);
struct client *client_next_with_pos(struct client *bc, Position p);
void client_swap(struct client *c1, struct client *c2);
void client_focus(struct client *c);
void client_get_name(struct client *c);
void client_close(struct client *c);
void uicb_client_close(Uicb cmd);
struct client *client_new(Window w, XWindowAttributes *wa);
void client_moveresize(struct client *c, struct geo g);
void client_maximize(struct client *c);
void client_fac_resize(struct client *c, Position p, int fac);
void client_remove(struct client *c);
void client_free(void);

/* Generated */
void uicb_client_resize_Right(Uicb);
void uicb_client_resize_Left(Uicb);
void uicb_client_resize_Top(Uicb);
void uicb_client_resize_Bottom(Uicb);
void uicb_client_focus_Right(Uicb);
void uicb_client_focus_Left(Uicb);
void uicb_client_focus_Top(Uicb);
void uicb_client_focus_Bottom(Uicb);
void uicb_client_swapsel_Right(Uicb);
void uicb_client_swapsel_Left(Uicb);
void uicb_client_swapsel_Top(Uicb);
void uicb_client_swapsel_Bottom(Uicb);
void uicb_client_focus_next(Uicb);
void uicb_client_focus_prev(Uicb);
void uicb_client_swapsel_next(Uicb);
void uicb_client_swapsel_prev(Uicb);

static inline struct client*
client_next(struct client *c)
{
     return (SLIST_NEXT(c, tnext)
               ? SLIST_NEXT(c, tnext)
               : SLIST_FIRST(&c->tag->clients));
}

static inline struct client*
client_prev(struct client *c)
{
     struct client *cc;

     for(cc = SLIST_FIRST(&c->tag->clients);
         SLIST_NEXT(cc, tnext) && SLIST_NEXT(cc, tnext) != c;
         cc = SLIST_NEXT(cc, tnext));

     return cc;
}

static inline bool
client_fac_geo(struct client *c, Position p, int fac)
{
     struct geo cg = c->geo;

     switch(p)
     {
          default:
          case Right:
               cg.w += fac;
               break;
          case Left:
               cg.x -= fac;
               cg.w += fac;
               break;
          case Top:
               cg.y -= fac;
               cg.h += fac;
               break;
          case Bottom:
               cg.h += fac;
               break;
     }

     /* Check for incompatible geo */
     if(cg.w > c->screen->ugeo.w || cg.h > c->screen->ugeo.h
               || cg.w < 3 || cg.h < 3)
          return false;

     /* Set transformed geo in tmp geo */
     c->tgeo = cg;

     return true;
}

static inline bool
client_fac_check_row(struct client *c, Position p, int fac)
{
     struct geo g = c->geo;
     struct client *cc;

     /* Travel clients to search parents of row and check geos */
     SLIST_FOREACH(cc, &c->tag->clients, tnext)
          if(GEO_PARENTROW(g, cc->geo, p) && !client_fac_geo(cc, p, fac))
               return false;

     return true;
}

static inline void
client_fac_arrange_row(struct client *c, Position p, int fac)
{
     struct geo g = c->geo;
     struct client *cc;

     /* Travel clients to search row parents and apply fac */
     SLIST_FOREACH(cc, &c->tag->clients, tnext)
          if(GEO_PARENTROW(g, cc->geo, p))
          {
               client_fac_geo(cc, p, fac);
               client_moveresize(cc, cc->tgeo);
          }
}

#endif /* CLIENT_H */
