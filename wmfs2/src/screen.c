/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "screen.h"
#include "util.h"

static Screen*
screen_new(Geo g)
{
     Screen *s = xcalloc(1, sizeof(Screen));

     s->geo = g;
     s->seltag = NULL;

     SLIST_INIT(&s->tags);

     SLIST_INSERT_HEAD(s, &W->h.screen, Screen, next);

     W->screen = s;

     return s;
}

void
screen_init(void)
{
     Geo g;

     g.x = 0;
     g.y = 0;
     g.w = DisplayWidth(W->dpy, W->xscreen);
     g.h = DisplayHeight(W->dpy, W->xscreen);

     SLIST_INIT(&W->h.screen);

     screen_new(g);
}

void
screen_free(void)
{
     FREE_LIST(Screen, W->h.screen);
}
