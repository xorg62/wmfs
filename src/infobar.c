/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "draw.h"
#include "infobar.h"
#include "barwin.h"
#include "util.h"
#include "tag.h"
#include "status.h"
#include "systray.h"
#include "client.h"

#define ELEM_FREE_BARWIN(e)                     \
     while(!SLIST_EMPTY(&e->bars))              \
     {                                          \
          b = SLIST_FIRST(&e->bars);            \
          SLIST_REMOVE_HEAD(&e->bars, enext);   \
          barwin_remove(b);                     \
     }

static void infobar_elem_tag_init(struct element *e);
static void infobar_elem_tag_update(struct element *e);
static void infobar_elem_status_init(struct element *e);
static void infobar_elem_status_update(struct element *e);
static void infobar_elem_systray_init(struct element *e);
static void infobar_elem_systray_update(struct element *e);
static void infobar_elem_launcher_init(struct element *e);
static void infobar_elem_launcher_update(struct element *e);

const struct elem_funcs
{
     char c;
     void (*func_init)(struct element *e);
     void (*func_update)(struct element *e);
} elem_funcs[] =
{
     { 't', infobar_elem_tag_init,      infobar_elem_tag_update },
     { 's', infobar_elem_status_init,   infobar_elem_status_update },
     { 'y', infobar_elem_systray_init,  infobar_elem_systray_update },
     { 'l', infobar_elem_launcher_init, infobar_elem_launcher_update },
     { '\0', NULL, NULL }
};

static void
infobar_elem_tag_init(struct element *e)
{
     struct tag *t;
     struct barwin *b, *prev = NULL;
     int s, j;

     /* Get final size before to use in placement */
     e->geo.w = e->infobar->theme->tags_border_width << 1;
     TAILQ_FOREACH(t, &e->infobar->screen->tags, next)
          e->geo.w += draw_textw(e->infobar->theme, t->name) + PAD;

     infobar_elem_placement(e);

     j = e->geo.x;
     e->geo.h -= (e->infobar->theme->tags_border_width << 1);

     e->statusctx = &e->infobar->theme->tags_n_sl;
     e->statusctx->flags |= STATUS_BLOCK_REFRESH;

     if(SLIST_EMPTY(&e->bars) || (e->infobar->screen->flags & SCREEN_TAG_UPDATE))
     {
          if((e->infobar->screen->flags & SCREEN_TAG_UPDATE))
          {
               ELEM_FREE_BARWIN(e);
               SLIST_INIT(&e->bars);
          }

          TAILQ_FOREACH(t, &e->infobar->screen->tags, next)
          {
               s = draw_textw(e->infobar->theme, t->name) + PAD;

               /* Init barwin */
               b = barwin_new(e->infobar->bar->win, j, 0, s, e->geo.h, e->infobar->theme->bars.fg, 0, false);

               /* Status doesn't have theme yet */
               t->statusctx.theme = e->infobar->theme;
               t->statusctx.flags |= STATUS_BLOCK_REFRESH;

               /* Set border */
               if(e->infobar->theme->tags_border_width)
               {
                    XSetWindowBorder(W->dpy, b->win, e->infobar->theme->tags_border_col);
                    XSetWindowBorderWidth(W->dpy, b->win, e->infobar->theme->tags_border_width);
               }

               b->ptr = (void*)t;
               barwin_map(b);

               b->mousebinds = W->tmp_head.tag;

               SLIST_INSERT_TAIL(&e->bars, b, enext, prev);

               prev = b;
               j += s;
          }
     }
     else
     {
          SLIST_FOREACH(b, &e->bars, enext)
          {
               barwin_move(b, j, 0);
               j += b->geo.w;
          }
     }
}

static void
infobar_elem_tag_update(struct element *e)
{
     struct tag *t, *sel = e->infobar->screen->seltag;
     struct barwin *b;

     SLIST_FOREACH(b, &e->bars, enext)
     {
          t = (struct tag*)b->ptr;

          /* Selected */
          if(t == sel)
          {
               b->fg = e->infobar->theme->tags_s.fg;
               b->bg = e->infobar->theme->tags_s.bg;
               e->statusctx = &e->infobar->theme->tags_s_sl;
          }
          else
          {
               /* Normal tag */
               if(SLIST_EMPTY(&t->clients))
               {
                    b->fg = e->infobar->theme->tags_n.fg;
                    b->bg = e->infobar->theme->tags_n.bg;
                    e->statusctx = &e->infobar->theme->tags_n_sl;
               }
               /* Urgent tag */
               else if(t->flags & TAG_URGENT)
               {
                    b->fg = e->infobar->theme->tags_u.fg;
                    b->bg = e->infobar->theme->tags_u.bg;
                    e->statusctx = &e->infobar->theme->tags_u_sl;
               }
               /* Occupied tag */
               else
               {
                    b->fg = e->infobar->theme->tags_o.fg;
                    b->bg = e->infobar->theme->tags_o.bg;
                    e->statusctx = &e->infobar->theme->tags_o_sl;
               }
          }

          barwin_refresh_color(b);

          /* Manage status line */
          e->statusctx->barwin = b;
          status_copy_mousebind(e->statusctx);
          status_render(e->statusctx);

          t->statusctx.barwin = b;
          status_copy_mousebind(&t->statusctx);
          status_render(&t->statusctx);

#ifdef HAVE_XFT
          draw_text(b->xftdraw, e->infobar->theme, (PAD >> 1),
                    TEXTY(e->infobar->theme, e->geo.h), b->fg, t->name);
#else
          draw_text(b->dr, e->infobar->theme, (PAD >> 1),
                    TEXTY(e->infobar->theme, e->geo.h), b->fg, t->name);
#endif /* HAVE_XFT */

          barwin_refresh(b);
     }
}

static void
infobar_elem_status_init(struct element *e)
{
     struct element *en = TAILQ_NEXT(e, next);
     struct barwin *b;

     infobar_elem_placement(e);

     e->geo.w = e->infobar->geo.w - e->geo.x - (en ? e->infobar->geo.w - en->geo.x : 0);

     if(!(b = SLIST_FIRST(&e->bars)))
     {
          b = barwin_new(e->infobar->bar->win, e->geo.x, 0, e->geo.w, e->geo.h, e->infobar->theme->bars.fg, 0, false);
          barwin_refresh_color(b);
          SLIST_INSERT_HEAD(&e->bars, b, enext);

          e->infobar->statusctx = status_new_ctx(b, e->infobar->theme);
          e->infobar->statusctx.status = strdup("wmfs2");
          e->infobar->statusctx.update = true;
     }
     else
     {
          barwin_move(b, e->geo.x, e->geo.y);
          barwin_resize(b, e->geo.w, e->geo.h);
     }

     b->fg = e->infobar->theme->bars.fg;
     b->bg = e->infobar->theme->bars.bg;

     barwin_map(b);
}

static void
infobar_elem_status_update(struct element *e)
{
     if(e->infobar->statusctx.update)
          status_manage(&e->infobar->statusctx);
     else
     {
          status_render(&e->infobar->statusctx);
          status_copy_mousebind(&e->infobar->statusctx);
     }
}

static void
infobar_elem_systray_init(struct element *e)
{
     struct barwin *b;

     /* Activate systray mask; no more systray element allowed now */
     W->flags |= WMFS_SYSTRAY;

     W->systray.infobar = e->infobar;

     e->geo.w = systray_get_width();
     infobar_elem_placement(e);

     if(!(b = SLIST_FIRST(&e->bars)))
     {
          b = barwin_new(e->infobar->bar->win, e->geo.x, 0, e->geo.w, e->geo.h, e->infobar->theme->bars.fg, 0, false);
          XFreePixmap(W->dpy, b->dr);
          SLIST_INSERT_HEAD(&e->bars, b, enext);
          W->systray.barwin = b;
          systray_acquire();
     }
     else
     {
          barwin_move(b, e->geo.x, e->geo.y);
          barwin_resize(b, e->geo.w, e->geo.h);
     }

     XMoveResizeWindow(W->dpy, W->systray.win, 0, 0, e->geo.w, e->geo.h);
}

static void
infobar_elem_systray_update(struct element *e)
{
     (void)e;

     systray_update();
}

static void
infobar_elem_launcher_init(struct element *e)
{
     struct barwin *b;

     if(!(W->flags & WMFS_LAUNCHER))
          e->geo.w = 1;

     infobar_elem_placement(e);

     if(!(b = SLIST_FIRST(&e->bars)))
     {
          b = barwin_new(e->infobar->bar->win, e->geo.x, 0, e->geo.w, e->geo.h, e->infobar->theme->bars.fg, 0, false);
          b->fg = e->infobar->theme->bars.fg;
          b->bg = e->infobar->theme->bars.bg;
          SLIST_INSERT_HEAD(&e->bars, b, enext);
     }
     else
     {
          barwin_move(b, e->geo.x, e->geo.y);
          barwin_resize(b, e->geo.w, e->geo.h);
     }

     barwin_refresh_color(b);
     barwin_refresh(b);
}

static void
infobar_elem_launcher_update(struct element *e)
{
     struct barwin *b = SLIST_FIRST(&e->bars);
     int l;

     if(!(W->flags & WMFS_LAUNCHER))
          return;

     barwin_refresh_color(b);

     l = draw_textw(e->infobar->theme, e->data) + 2;
#ifdef HAVE_XFT
     draw_text(b->xftdraw, e->infobar->theme, 1, TEXTY(e->infobar->theme, e->geo.h), b->fg, e->data);
#else
     draw_text(b->dr, e->infobar->theme, 1, TEXTY(e->infobar->theme, e->geo.h), b->fg, e->data);
#endif /* HAVE_XFT */

     /* Cursor */
     XDrawLine(W->dpy, b->dr, W->gc, l, 2, l, e->geo.h - 4);

     barwin_refresh(b);
}

#define ELEM_INIT(a)                                  \
     do {                                             \
          e = xcalloc(1, sizeof(struct element));     \
          SLIST_INIT(&e->bars);                       \
          e->infobar = i;                             \
          e->type = j;                                \
          e->data = NULL;                             \
          e->align = a;                               \
          e->func_init = elem_funcs[j].func_init;     \
          e->func_update = elem_funcs[j].func_update; \
     } while(/* CONSTCOND */ 0);
static void
infobar_elem_init(struct infobar *i)
{
     struct element *e, *es = NULL;
     int n, j, k, l = (int)strlen(i->elemorder);
     bool s = false;

     TAILQ_INIT(&i->elements);

     for(n = 0; n < l; ++n)
     {
          /* Element status found, manage other element from the end */
          if(i->elemorder[n] == 's')
          {
               s = true;
               ++n;
               break;
          }

          /* Only one systray element in a wmfs session */
          if(i->elemorder[n] == 'y' && W->flags & WMFS_SYSTRAY)
               continue;

          for(j = 0; j < (int)LEN(elem_funcs); ++j)
               if(elem_funcs[j].c == i->elemorder[n])
               {
                    ELEM_INIT(Left);

                    TAILQ_INSERT_TAIL(&i->elements, e, next);

                    e->func_init(e);
                    es = e;

                    break;
               }
     }

     /* Status make next elements aligning to the right */
     if(s)
     {
          /* Manage element from the end */
          for(k = l - 1; k >= n; --k)
          {
               /* Only one status */
               if(i->elemorder[k] == 's' || (i->elemorder[n] == 'y' && W->flags & WMFS_SYSTRAY))
                    continue;

               for(j = 0; j < (int)LEN(elem_funcs); ++j)
                    if(elem_funcs[j].c == i->elemorder[k])
                    {
                         ELEM_INIT(Right);

                         if(es)
                              TAILQ_INSERT_AFTER(&i->elements, es, e, next);
                         else
                              TAILQ_INSERT_HEAD(&i->elements, e, next);

                         e->func_init(e);

                         break;
                    }
          }

          /* Init status at the end */
          j = ElemStatus;
          ELEM_INIT(Left);

          if(es)
               TAILQ_INSERT_AFTER(&i->elements, es, e, next);
          else
               TAILQ_INSERT_HEAD(&i->elements, e, next);

          e->func_init(e);
     }
}

void
infobar_elem_update(struct infobar *i, int type)
{
     struct element *e;

     TAILQ_FOREACH(e, &i->elements, next)
          if(type == e->type || type == -1)
               e->func_update(e);
}

void
infobar_elem_reinit(struct infobar *i)
{
     struct element *e;

     barwin_refresh_color(i->bar);

     TAILQ_FOREACH(e, &i->elements, next)
     {
          /* Status element found, scan from the tail now */
          if(e->type == ElemStatus)
          {
               struct element *ee;

               TAILQ_FOREACH_REVERSE(ee, &i->elements, esub, next)
               {
                    if(e == ee)
                         break;

                    ee->func_init(ee);
                    ee->func_update(ee);
               }

               e->func_init(e);
               e->func_update(e);

               return;
          }

          e->func_init(e);
          e->func_update(e);
     }

     barwin_refresh(i->bar);
}

struct infobar*
infobar_new(struct screen *s, char *name, struct theme *theme, enum barpos pos, const char *elem)
{
     bool map;
     struct infobar *i = (struct infobar*)xcalloc(1, sizeof(struct infobar));

     i->screen = s;
     i->theme = theme;
     i->elemorder = xstrdup(elem);
     i->name = xstrdup(name);

     map = infobar_placement(i, pos);

     /* struct barwin create */
     i->bar = barwin_new(W->root, i->geo.x, i->geo.y, i->geo.w, i->geo.h,
                         theme->bars.fg, theme->bars.bg, false);

     SLIST_INSERT_HEAD(&s->infobars, i, next);

     /* struct elements */
     infobar_elem_init(i);

     /* Render, only if pos is Top or Bottom */
     if(!map)
          return i;

     barwin_map(i->bar);
     barwin_map_subwin(i->bar);
     barwin_refresh_color(i->bar);
     infobar_refresh(i);

     return i;
}

void
infobar_refresh(struct infobar *i)
{
     infobar_elem_update(i, -1);

     barwin_refresh(i->bar);
}

void
infobar_remove(struct infobar *i)
{
     struct element *e;
     struct barwin *b;

     free(i->elemorder);
     free(i->name);

     if(i == W->systray.infobar)
          systray_freeicons();

     while(!TAILQ_EMPTY(&i->elements))
     {
          e = TAILQ_FIRST(&i->elements);
          TAILQ_REMOVE(&i->elements, e, next);
          ELEM_FREE_BARWIN(e);
          free(e);
     }

     barwin_remove(i->bar);

     SLIST_REMOVE(&i->screen->infobars, i, infobar, next);

     free(i);
}

void
infobar_free(struct screen *s)
{
     struct infobar *i;

     while(!SLIST_EMPTY(&s->infobars))
     {
          i = SLIST_FIRST(&s->infobars);

          /* SLIST_REMOVE is done by infobar_remove */
          infobar_remove(i);
     }
}

void
uicb_infobar_toggle_hide(Uicb iname)
{
     struct client *c;
     struct infobar *i;

     if(iname)
          i = infobar_gb_name(iname);
     else
          i = SLIST_FIRST(&W->screen->infobars);

     if(i->pos == BarHide)
     {
          i->pos = i->opos;

          if(infobar_placement(i, i->pos))
          {
               barwin_map(i->bar);
               barwin_map_subwin(i->bar);
               barwin_refresh_color(i->bar);
               infobar_refresh(i);
          }
     }
     else
     {
          i->opos = i->pos;
          i->pos = BarHide;

          barwin_unmap_subwin(i->bar);
          barwin_unmap(i->bar);

          switch(i->opos)
          {
               case BarTop:
                    i->screen->ugeo.y -= i->geo.h;
               case BarBottom:
                    i->screen->ugeo.h += i->geo.h;
               case BarHide:
               default:
                    break;
          }
     }

     SLIST_FOREACH(c, &W->h.client, next)
          layout_fix_hole(c);
}
