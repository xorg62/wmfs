/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef LAYOUT_H
#define LAYOUT_H

#include "wmfs.h"

/* Check lateral direction (if p is Right or Left) */
#define LDIR(P) (P < Top)

/* Reverse position */
#define RPOS(P) (P & 1 ? P - 1 : P + 1)

/* geo comparaison */
#define GEO_CHECK2(g1, g2, p) (LDIR(p) ? ((g1).h == (g2).h) : ((g1).w == (g2).w))
#define GEO_CHECK_ROW(g1, g2, p)                                     \
     (LDIR(p)                                                        \
      ? ((g1).y >= (g2).y && ((g1).y + (g1).h) <= ((g2).y + (g2).h)) \
      : ((g1).x >= (g2).x && ((g1).x + (g1).w) <= ((g2).x + (g2).w)))
#define GEO_PARENTROW(g1, g2, p)                                                \
     (LDIR(p)                                                                   \
      ? (p == Left ? ((g1).x == (g2).x) : ((g1).x + (g1).w == (g2).x + (g2).w)) \
      : (p == Top  ? ((g1).y == (g2).y) : ((g1).y + (g1).h == (g2).y + (g2).h)))


/* Debug */
#define DGEO(G) printf(": %d %d %d %d\n", G.x, G.y, G.w, G.h)

void layout_save_set(struct tag *t);
void layout_free_set(struct tag *t);
void layout_split_integrate(struct client *c, struct client *sc);
void layout_split_arrange_closed(struct client *ghost);
void layout_fix_hole(struct client *c);
void layout_client(struct client *c);
void uicb_layout_vmirror(Uicb cmd);
void uicb_layout_hmirror(Uicb cmd);
void uicb_layout_rotate_left(Uicb cmd);
void uicb_layout_rotate_right(Uicb cmd);
void uicb_layout_prev_set(Uicb cmd);
void uicb_layout_next_set(Uicb cmd);

/* Generated */
void uicb_layout_integrate_Left(Uicb);
void uicb_layout_integrate_Right(Uicb);
void uicb_layout_integrate_Top(Uicb);
void uicb_layout_integrate_Bottom(Uicb);


#endif /* LAYOUT_H */

