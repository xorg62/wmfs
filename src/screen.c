/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* HAVE_XINERAMA */

#include "screen.h"
#include "util.h"
#include "tag.h"
#include "infobar.h"
#include "client.h"

static struct screen*
screen_new(struct geo *g, int id)
{
     struct screen *s = (struct screen*)xcalloc(1, sizeof(struct screen));

     s->geo = s->ugeo = *g;
     s->seltag = NULL;
     s->id = id;

     TAILQ_INIT(&s->tags);
     SLIST_INIT(&s->infobars);

     SLIST_INSERT_HEAD(&W->h.screen, s, next);

     /* Set as selected screen */
     W->screen = s;

     return s;
}

void
screen_init(void)
{
     struct geo g;

     SLIST_INIT(&W->h.screen);

#ifdef HAVE_XINERAMA
     XineramaScreenInfo *xsi;
     int i, n = 0;

     if(XineramaIsActive(W->dpy))
     {
          xsi = XineramaQueryScreens(W->dpy, &n);

          for(i = 0; i < n; ++i)
          {
               g.x = xsi[i].x_org;
               g.y = xsi[i].y_org;
               g.w = xsi[i].width;
               g.h = xsi[i].height;

               screen_new(&g, i);
          }

          XFree(xsi);
     }
     else
#endif /* HAVE_XINERAMA */
     {
          g.x = g.y = 0;
          g.w = DisplayWidth(W->dpy, W->xscreen);
          g.h = DisplayHeight(W->dpy, W->xscreen);

          screen_new(&g, 0);
     }
}

/*
 * Update selected screen with mouse location
 */
struct screen*
screen_update_sel(void)
{
#ifdef HAVE_XINERAMA
     if(XineramaIsActive(W->dpy))
     {
          struct screen *s;
          Window w;
          int d, x, y;

          XQueryPointer(W->dpy, W->root, &w, &w, &x, &y, &d, &d, (unsigned int *)&d);

          SLIST_FOREACH(s, &W->h.screen, next)
               if(INAREA(x, y, s->geo))
                    break;

          if(!s)
               s = SLIST_FIRST(&W->h.screen);

          return (W->screen = s);
     }
#endif /* HAVE_XINERAMA */

     return W->screen;
}

void
screen_free(void)
{
     struct screen *s;

     while(!SLIST_EMPTY(&W->h.screen))
     {
          s = SLIST_FIRST(&W->h.screen);
          SLIST_REMOVE_HEAD(&W->h.screen, next);

          infobar_free(s);
          tag_free(s);
          free(s);
     }
}
