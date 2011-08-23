/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#define HAVE_XINERAMA

#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* HAVE_XINERAMA */

#include "screen.h"
#include "util.h"

static Scr33n*
screen_new(Geo *g)
{
     Scr33n *s = xcalloc(1, sizeof(Scr33n));

     s->geo = *g;
     s->seltag = NULL;

     SLIST_INIT(&s->tags);

     SLIST_INSERT_HEAD(&W->h.screen, s, next);

     /* Set as selected screen */
     W->screen = s;

     return s;
}

void
screen_init(void)
{
     Scr33n *s;
     Geo g;

     SLIST_INIT(&W->h.screen);

#ifdef HAVE_XINERAMA
     XineramaScreenInfo *xsi;
     int i = 0, n;

     if(XineramaIsActive(W->dpy))
     {
          xsi = XineramaQueryScreens(W->dpy, &n);

          for(; i < n; ++i)
          {
               s = NULL;
               g.x = xsi[i].x_org;
               g.y = xsi[i].y_org;
               g.w = xsi[i].width;
               g.h = xsi[i].height;

               s = screen_new(&g);
               tag_screen(s, tag_new(s, "tag")); /* tmp */

               SLIST_INSERT_HEAD(&W->h.screen, s, next);

               printf("%d: %d %d %d %d\n", i, s->geo.x, s->geo.y, s->geo.w, s->geo.h);
          }

          XFree(xsi);
     }
     else
#endif /* HAVE_XINERAMA */
     {
          g.x = g.y = 0;
          g.w = DisplayWidth(W->dpy, W->xscreen);
          g.h = DisplayHeight(W->dpy, W->xscreen);

          s = screen_new(&g);
          tag_screen(s, tag_new(s, "tag"));

          SLIST_INSERT_HEAD(&W->h.screen, s, next);
          printf("%d: %d %d %d %d\n", i, s->geo.x, s->geo.y, s->geo.w, s->geo.h);
     }

}

void
screen_free(void)
{
     FREE_LIST(Scr33n, W->h.screen);
}
