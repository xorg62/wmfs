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

          W->nscreen = n;

          XFree(xsi);
     }
     else
#endif /* HAVE_XINERAMA */
     {
          g.x = g.y = 0;
          g.w = DisplayWidth(W->dpy, W->xscreen);
          g.h = DisplayHeight(W->dpy, W->xscreen);

          screen_new(&g, 0);
          W->nscreen = 1;
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
          return (W->screen = screen_gb_mouse());
#endif /* HAVE_XINERAMA */

     return W->screen;
}

static void
screen_select(struct screen *s)
{
     XWarpPointer(W->dpy, None, W->root, 0, 0, 0, 0,
                  s->ugeo.x + (s->ugeo.w >> 1),
                  s->ugeo.y + (s->ugeo.h >> 1));

     W->screen = s;
}

void
uicb_screen_next(Uicb cmd)
{
     struct screen *s;
     (void)cmd;

     if(!(s = SLIST_NEXT(W->screen, next)))
          s = SLIST_FIRST(&W->h.screen);

     screen_select(s);
}

void
uicb_screen_prev(Uicb cmd)
{
     struct screen *s;
     (void)cmd;

     SLIST_FOREACH(s, &W->h.screen, next)
          if(SLIST_NEXT(W->screen, next) == s)
          {
               screen_select(s);
               return;
          }

     screen_select(SLIST_FIRST(&W->h.screen));
}

static void
screen_move_client(struct client *c, struct screen *s)
{
     if (c && s) {
          c->screen = s;
          tag_client(s->seltag, c);
     }
}

void
uicb_screen_move_client_next(Uicb cmd)
{
     struct screen *s;
     (void)cmd;

     if(!(s = SLIST_NEXT(W->screen, next)))
          s = SLIST_FIRST(&W->h.screen);

     screen_move_client(W->client, s);
}

void
uicb_screen_move_client_prev(Uicb cmd)
{
     struct screen *s;
     (void)cmd;

     SLIST_FOREACH(s, &W->h.screen, next)
          if(SLIST_NEXT(W->screen, next) == s)
          {
               screen_move_client(W->client, s);
               return;
          }

     screen_move_client(W->client, SLIST_FIRST(&W->h.screen));
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
