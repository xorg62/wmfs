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

static void infobar_elem_tag_init(struct element *e);
static void infobar_elem_tag_update(struct element *e);
static void infobar_elem_status_init(struct element *e);
static void infobar_elem_status_update(struct element *e);

const struct elem_funcs
{
     char c;
     void (*func_init)(struct element *e);
     void (*func_update)(struct element *e);
} elem_funcs[] =
{
     { 't', infobar_elem_tag_init,    infobar_elem_tag_update },
     { 's', infobar_elem_status_init, infobar_elem_status_update },

     /* { 'l',  infobar_elem_layout_init, infobar_elem_layout_update },
        { 'S',  infobar_elem_selbar_init, infobar_elem_selbar_update },
     */
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

     TAILQ_FOREACH(t, &e->infobar->screen->tags, next)
     {
          s = draw_textw(e->infobar->theme, t->name) + PAD;

          /* Init barwin */
          b = barwin_new(e->infobar->bar->win, j, 0, s, e->geo.h, 0, 0, false);

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

     e->infobar->screen->elemupdate |= FLAGINT(ElemTag);
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
          }
          else
          {
               b->fg = e->infobar->theme->tags_n.fg;
               b->bg = e->infobar->theme->tags_n.bg;
          }

          barwin_refresh_color(b);

          draw_text(b->dr, e->infobar->theme, (PAD >> 1),
                    TEXTY(e->infobar->theme, e->geo.h), b->fg, t->name);

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

     b = barwin_new(e->infobar->bar->win, e->geo.x, 0, e->geo.w, e->geo.h, 0, 0, false);

     b->fg = e->infobar->theme->bars.fg;
     b->bg = e->infobar->theme->bars.bg;

     barwin_map(b);

     SLIST_INSERT_HEAD(&e->bars, b, enext);

     e->infobar->statusctx = status_new_ctx(b, e->infobar->theme);

     e->infobar->screen->elemupdate |= FLAGINT(ElemStatus);
     e->infobar->statusctx.status = strdup("wmfs2");
}

static void
infobar_elem_status_update(struct element *e)
{
     status_manage(&e->infobar->statusctx);
}

#define ELEM_INIT(a)                                  \
     do {                                             \
          e = xcalloc(1, sizeof(struct element));     \
          SLIST_INIT(&e->bars);                       \
          e->infobar = i;                             \
          e->type = j;                                \
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

          for(j = 0; j < (int)LEN(elem_funcs); ++j)
               if(elem_funcs[j].c == i->elemorder[n])
               {
                    ELEM_INIT(Left);

                    if(TAILQ_EMPTY(&i->elements))
                         TAILQ_INSERT_HEAD(&i->elements, e, next);
                    else
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
               if(i->elemorder[k] == 's')
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
infobar_elem_update(struct infobar *i)
{
     struct element *e;

     TAILQ_FOREACH(e, &i->elements, next)
          if(i->screen->elemupdate & FLAGINT(e->type))
               e->func_update(e);
}

void
infobar_elem_remove(struct element *e)
{
     struct barwin *b;

     TAILQ_REMOVE(&e->infobar->elements, e, next);

     while(!SLIST_EMPTY(&e->bars))
     {
          b = SLIST_FIRST(&e->bars);
          SLIST_REMOVE_HEAD(&e->bars, enext);
          barwin_remove(b);
     }

     free(e);
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
     infobar_elem_update(i);

     barwin_refresh(i->bar);
}

void
infobar_remove(struct infobar *i)
{
     struct element *e;

     free(i->elemorder);
     free(i->name);
     free(i->status);

     TAILQ_FOREACH(e, &i->elements, next)
          infobar_elem_remove(e);

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






