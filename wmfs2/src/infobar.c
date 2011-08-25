/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "draw.h"
#include "infobar.h"
#include "barwin.h"
#include "util.h"


#define ELEM_DEFAULT_ORDER "tlsS"
#define INFOBAR_DEF_W (12)

void
infobar_init(void)
{
     Infobar *i;
     Scr33n *s;

     SLIST_FOREACH(s, &W->h.screen, next)
     {
          i = (Infobar*)xcalloc(1, sizeof(Infobar));

          i->screen = s;
          i->elemorder = xstrdup(ELEM_DEFAULT_ORDER);
          STAILQ_INIT(&i->elements);

          /* Positions TODO: geo = infobar_position(Position {Top,Bottom,Hidden}) */
          i->geo = s->geo;
          i->geo.h = INFOBAR_DEF_W;

          /* Barwin create */
          i->bar = barwin_new(W->root, i->geo.x, i->geo.y, i->geo.w, i->geo.h, 0x222222, 0xCCCCCC, false);

          /* Render */
          barwin_map(i->bar);
          barwin_map_subwin(i->bar);
          barwin_refresh_color(i->bar);

          /* TODO: infobar_elem_init(i) */
          infobar_refresh(i);

          SLIST_INSERT_HEAD(&s->infobars, i, next);
          i = NULL;
     }
}

void
infobar_refresh(Infobar *i)
{
     draw_text(i->bar->dr, 1, TEXTY(INFOBAR_DEF_W), 0x005500, "WMFS2");

     barwin_refresh(i->bar);
}

void
infobar_remove(Infobar *i)
{
     free(i->elemorder);

     /* TODO: infobar_elem_free */
     barwin_remove(i->bar);

     SLIST_REMOVE(&i->screen->infobars, i, Infobar, next);

     free(i);
}

void
infobar_free(Scr33n *s)
{
     Infobar *i;

     while(!SLIST_EMPTY(&s->infobars))
     {
          i = SLIST_FIRST(&s->infobars);
          infobar_remove(i);
     }
}






