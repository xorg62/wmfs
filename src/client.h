/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "wmfs.h"
#include "layout.h"

#define TCLIENT_CHECK(C) (C->flags & CLIENT_TABBED && !(C->flags & CLIENT_TABMASTER))

inline void client_configure(struct client *c);
struct client *client_gb_win(Window w);
struct client *client_gb_frame(Window w);
struct client *client_gb_pos(struct tag *t, int x, int y);
struct client *client_next_with_pos(struct client *bc, enum position p);
void client_swap2(struct client *c1, struct client *c2);
void client_swap(struct client *c, enum position p);
void client_focus(struct client *c);
void client_get_name(struct client *c);
void client_close(struct client *c);
void uicb_client_close(Uicb cmd);
struct client *client_new(Window w, XWindowAttributes *wa, bool scan);
bool client_winsize(struct client *c, struct geo *geo);
void client_moveresize(struct client *c, struct geo *g);
void client_maximize(struct client *c);
void client_fac_resize(struct client *c, enum position p, int fac);
void client_fac_adjust(struct client *c);
void client_remove(struct client *c);
void client_free(void);
void _fac_resize(struct client *c, enum position p, int fac);
void client_apply_tgeo(struct tag *t);

#define CPROP_LOC  0x01
#define CPROP_FLAG 0x02
#define CPROP_GEO  0x04
void client_update_props(struct client *c, Flags f);

inline void client_fac_hint(struct client *c);

/* Generated */
void uicb_client_resize_Right(Uicb);
void uicb_client_resize_Left(Uicb);
void uicb_client_resize_Top(Uicb);
void uicb_client_resize_Bottom(Uicb);
void uicb_client_focus_Right(Uicb);
void uicb_client_focus_Left(Uicb);
void uicb_client_focus_Top(Uicb);
void uicb_client_focus_Bottom(Uicb);
void uicb_client_tab_Right(Uicb);
void uicb_client_tab_Left(Uicb);
void uicb_client_tab_Top(Uicb);
void uicb_client_tab_Bottom(Uicb);
void uicb_client_swap_Right(Uicb);
void uicb_client_swap_Left(Uicb);
void uicb_client_swap_Top(Uicb);
void uicb_client_swap_Bottom(Uicb);
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
     struct client *cc = SLIST_FIRST(&c->tag->clients);

     while(SLIST_NEXT(cc, tnext) && SLIST_NEXT(cc, tnext) != c)
          cc = SLIST_NEXT(cc, tnext);

     return cc;
}

#endif /* CLIENT_H */
