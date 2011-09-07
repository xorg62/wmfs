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

static void infobar_elem_tag_init(struct Element *e);
static void infobar_elem_tag_update(struct Element *e);

const struct elem_funcs
{
     char c;
     void (*func_init)(struct Element *e);
     void (*func_update)(struct Element *e);
} elem_funcs[] =
{
     { 't', infobar_elem_tag_init, infobar_elem_tag_update },

     /* { 'l',  infobar_elem_layout_init, infobar_elem_layout_update },
        { 's',  infobar_elem_selbar_init, infobar_elem_selbar_update },
        { 'S',  infobar_elem_status_init, infobar_elem_status_update },*/

     { '\0', NULL, NULL }
};

static void
infobar_elem_tag_init(struct Element *e)
{
     struct Tag *t;
     struct Barwin *b, *prev;
     struct Geo g = { 0, 0, 0, 0 };
     int s, j;

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

          /* TODO: refer to tag element configuration */
          barwin_mousebind_new(b, Button1, false, g, uicb_tag_set_with_name, (Uicb)t->name);
          barwin_mousebind_new(b, Button4, false, g, uicb_tag_next, NULL);
          barwin_mousebind_new(b, Button5, false, g, uicb_tag_prev, NULL);

          /* insert_tail with SLIST */
          if(SLIST_EMPTY(&e->bars))
               SLIST_INSERT_HEAD(&e->bars, b, enext);
          else
               SLIST_INSERT_AFTER(prev, b, enext);

          prev = b;
          j += s;
     }

     e->infobar->screen->elemupdate |= FLAGINT(ElemTag);

     e->geo.w = j;
}

static void
infobar_elem_tag_update(struct Element *e)
{
     struct Tag *t, *sel = e->infobar->screen->seltag;
     struct Barwin *b;

     SLIST_FOREACH(b, &e->bars, enext)
     {
          t = (struct Tag*)b->ptr;

          /* Selected */
          /* TODO: color from conf */
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
infobar_elem_init(struct Infobar *i)
{
     struct Element *e;
     int n, j;

     TAILQ_INIT(&i->elements);

     for(n = 0; n < strlen(i->elemorder); ++n)
     {
          for(j = 0; j < LEN(elem_funcs); ++j)
               if(elem_funcs[j].c == i->elemorder[n])
               {
                    e = xcalloc(1, sizeof(struct Element));

                    SLIST_INIT(&e->bars);

                    e->infobar = i;
                    e->type = j;
                    e->func_init = elem_funcs[j].func_init;
                    e->func_update = elem_funcs[j].func_update;

                    TAILQ_INSERT_TAIL(&i->elements, e, next);

                    e->func_init(e);

                    break;
               }
     }
}

void
infobar_elem_update(struct Infobar *i)
{
     struct Element *e;

     TAILQ_FOREACH(e, &i->elements, next)
          if(i->screen->elemupdate & FLAGINT(e->type))
               e->func_update(e);
}

void
infobar_elem_remove(struct Element *e)
{
     struct Barwin *b;

     TAILQ_REMOVE(&e->infobar->elements, e, next);

     while(!SLIST_EMPTY(&e->bars))
     {
          b = SLIST_FIRST(&e->bars);
          SLIST_REMOVE_HEAD(&e->bars, enext);
          barwin_remove(b);
     }
}

struct Infobar*
infobar_new(struct Scr33n *s, struct Theme *theme, Barpos pos, const char *elem)
{
     bool map;
     int n;

     struct Infobar *i = (struct Infobar*)xcalloc(1, sizeof(struct Infobar));

     i->screen = s;
     i->theme = theme;
     i->elemorder = xstrdup(elem);

     map = infobar_placement(i, pos);

     /* struct Barwin create */
     i->bar = barwin_new(W->root, i->geo.x, i->geo.y, i->geo.w, i->geo.h,
               theme->bars.fg, theme->bars.bg, false);

     SLIST_INSERT_HEAD(&s->infobars, i, next);

     /* struct Elements */
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
infobar_refresh(struct Infobar *i)
{
     infobar_elem_update(i);

     barwin_refresh(i->bar);
}

void
infobar_remove(struct Infobar *i)
{
     struct Element *e;

     free(i->elemorder);

     TAILQ_FOREACH(e, &i->elements, next)
          infobar_elem_remove(e);

     barwin_remove(i->bar);

     SLIST_REMOVE(&i->screen->infobars, i, Infobar, next);

     free(i);
}

void
infobar_free(struct Scr33n *s)
{
     struct Infobar *i;

     while(!SLIST_EMPTY(&s->infobars))
     {
          i = SLIST_FIRST(&s->infobars);

          /* SLIST_REMOVE is done by infobar_remove */
          infobar_remove(i);
     }
}






