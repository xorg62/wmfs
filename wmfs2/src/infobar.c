/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <math.h>

#include "wmfs.h"
#include "draw.h"
#include "infobar.h"
#include "barwin.h"
#include "util.h"
#include "tag.h"

static void infobar_elem_tag_init(Element *e);
static void infobar_elem_tag_update(Element *e);

const struct elem_funcs
{
     char c;
     void (*func_init)(Element *e);
     void (*func_update)(Element *e);
} elem_funcs[] =
{
     { 't', infobar_elem_tag_init, infobar_elem_tag_update },

     /* { 'l',  infobar_elem_layout_init, infobar_elem_layout_update },
        { 's',  infobar_elem_selbar_init, infobar_elem_selbar_update },
        { 'S',  infobar_elem_status_init, infobar_elem_status_update },*/

     { '\0', NULL, NULL }
};

static void
infobar_elem_tag_init(Element *e)
{
     Tag *t;
     Barwin *b, *prev;
     Geo g = { 0, 0, 0, 0 };
     int s, j;

     infobar_elem_placement(e);

     j = e->geo.x;
     e->geo.h -= (e->infobar->theme->tags_border_width << 1);

     TAILQ_FOREACH(t, &e->infobar->screen->tags, next)
     {
          s = draw_textw(t->name) + PAD;

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
infobar_elem_tag_update(Element *e)
{
     Tag *t, *sel = e->infobar->screen->seltag;
     Barwin *b;

     SLIST_FOREACH(b, &e->bars, enext)
     {
          t = (Tag*)b->ptr;

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

          draw_text(b->dr, (PAD >> 1), TEXTY(e->geo.h), b->fg, t->name);

          barwin_refresh(b);
     }

}

static void
infobar_elem_init(Infobar *i)
{
     Element *e;
     int n, j;

     TAILQ_INIT(&i->elements);

     for(n = 0; n < strlen(i->elemorder); ++n)
     {
          for(j = 0; j < LEN(elem_funcs); ++j)
               if(elem_funcs[j].c == i->elemorder[n])
               {
                    e = xcalloc(1, sizeof(Element));

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
infobar_elem_update(Infobar *i)
{
     Element *e;

     TAILQ_FOREACH(e, &i->elements, next)
          if(i->screen->elemupdate & FLAGINT(e->type))
               e->func_update(e);
}

void
infobar_elem_remove(Element *e)
{
     Barwin *b;

     TAILQ_REMOVE(&e->infobar->elements, e, next);

     while(!SLIST_EMPTY(&e->bars))
     {
          b = SLIST_FIRST(&e->bars);
          SLIST_REMOVE_HEAD(&e->bars, enext);
          barwin_remove(b);
     }
}

Infobar*
infobar_new(Scr33n *s, Theme *theme, const char *elem)
{
     int n;

     Infobar *i = (Infobar*)xcalloc(1, sizeof(Infobar));

     i->screen = s;
     i->theme = theme;
     i->elemorder = xstrdup(elem);

     /* TODO: Position top/bottom */
     infobar_placement(i);

     /* Barwin create */
     i->bar = barwin_new(W->root, i->geo.x, i->geo.y, i->geo.w, i->geo.h,
               theme->bars.fg, theme->bars.bg, false);

     /* Render */
     barwin_map(i->bar);
     barwin_map_subwin(i->bar);
     barwin_refresh_color(i->bar);

     /* Elements */
     infobar_elem_init(i);

     infobar_refresh(i);

     SLIST_INSERT_HEAD(&s->infobars, i, next);

     return i;
}

void
infobar_refresh(Infobar *i)
{
     infobar_elem_update(i);

     barwin_refresh(i->bar);
}

void
infobar_remove(Infobar *i)
{
     Element *e;

     free(i->elemorder);

     TAILQ_FOREACH(e, &i->elements, next)
          infobar_elem_remove(e);

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






