/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "status.h"
#include "barwin.h"
#include "config.h"
#include "infobar.h"
#include "util.h"

#include <string.h>

static struct status_seq*
status_new_seq(char type, int narg, int minarg, char *args[], int *shift)
{
     struct status_seq *sq = xcalloc(1, sizeof(struct status_seq));

     SLIST_INIT(&sq->mousebinds);
     sq->type = type;
     *shift = 0;

     if(narg == minarg ||  !strcmp(args[0], "right") || !strcmp(args[0], "left"))
          sq->align = str_to_position(args[0]);
     else
     {
          sq->align = NoAlign;
          sq->geo.x = ATOI(args[0]);
          sq->geo.y = ATOI(args[1]);
          *shift = 1;
     }

     return sq;
}

/* Parse mousebind sequence next normal sequence: \<seq>[](button;func;cmd) */
static char*
status_parse_mouse(struct status_seq *sq, struct element *e, char *str)
{
     struct mousebind *m;
     struct barwin *b = SLIST_FIRST(&e->bars);
     char *end, *arg[3] = { NULL };
     int i;

     if(*str != '(' || !(end = strchr(str, ')')))
          return str + 1;

     i = parse_args(++str, ';', ')', 3, arg);

     m = xcalloc(1, sizeof(struct mousebind));

     m->use_area = true;
     m->button   = ATOI(arg[0]);
     m->func     = uicb_name_func(arg[1]);
     m->cmd      = (i > 1 ? xstrdup(arg[2]) : NULL);

     SLIST_INSERT_HEAD(&b->mousebinds, m, next);
     SLIST_INSERT_HEAD(&sq->mousebinds, m, snext);

     return end + 1;
}

#define STATUS_CHECK_ARGS(i, n1, n2, str, end)  \
     if(i != n1 && i != n2)                     \
     {                                          \
          str = end + 2;                        \
          continue;                             \
     }
static void
status_parse(struct element *e)
{
     struct status_seq *sq, *prev = NULL;
     int i, shift = 0;
     char *dstr = xstrdup(e->infobar->status), *sauv = dstr;
     char type, *p, *end, *arg[6] = { NULL };

     for(; *dstr; ++dstr)
     {
          /* Check if this is a sequence */
          if(*dstr != '\\')
               continue;

          p = ++dstr;

          if(!(strchr("sR", *p)) || !(end = strchr(p, ']')))
               continue;

          /* Then parse & list it */
          switch((type = *p))
          {
          /*
           * Text sequence: \s[left/right;#color;text] OR \s[x;y;#color;text]
           */
          case 's':
               i = parse_args(p + 2,  ';', ']', 4, arg);
               STATUS_CHECK_ARGS(i, 2, 3, dstr, end);
               sq = status_new_seq(type, i, 2, arg, &shift);

               sq->color = color_atoh(arg[1 + shift]);
               sq->str = xstrdup(arg[2 + shift]);

               break;

         /*
          * Rectangle sequence: \R[left/right;w;h;#color] OR \R[x;y;w;h;#color]
          */
          case 'R':
               i = parse_args(p + 2, ';', ']', 5, arg);
               STATUS_CHECK_ARGS(i, 3, 4, dstr, end);
               sq = status_new_seq(type, i, 3, arg, &shift);

               sq->geo.w = ATOI(arg[1 + shift]);
               sq->geo.h = ATOI(arg[2 + shift]);
               sq->color = color_atoh(arg[3 + shift]);

               break;
          }

          SLIST_INSERT_TAIL(&e->infobar->statushead, sq, next, prev);

          /*
           * Optional mousebind sequence(s) \<seq>[](button;func;cmd)
           * Parse it while there is a mousebind sequence.
           */
          dstr = ++end;
          while((*(dstr = status_parse_mouse(sq, e, dstr)) == '('));

          prev = sq;
     }

     free(sauv);
}

#define STATUS_ALIGN(align)                             \
     if(align == Left)                                  \
     {                                                  \
          sq->geo.x = left;                             \
          left += sq->geo.w;                            \
     }                                                  \
     else if(align == Right)                            \
     {                                                  \
          sq->geo.x = e->geo.w - right - sq->geo.w;     \
          right += sq->geo.w;                           \
     }
static void
status_apply_list(struct element *e)
{
     struct status_seq *sq;
     struct barwin *b = SLIST_FIRST(&e->bars);
     struct mousebind *m;
     int left = 0, right = 0;

     SLIST_FOREACH(sq, &e->infobar->statushead, next)
     {
          switch(sq->type)
          {
          /* Text */
          case 's':
               sq->geo.w = draw_textw(e->infobar->theme, sq->str);
               sq->geo.h = e->infobar->theme->font.height;

               if(sq->align != NoAlign)
                    sq->geo.y = TEXTY(e->infobar->theme, e->geo.h);

               STATUS_ALIGN(sq->align);

               draw_text(b->dr, e->infobar->theme, sq->geo.x, sq->geo.y, sq->color, sq->str);

               if(!SLIST_EMPTY(&sq->mousebinds))
                    SLIST_FOREACH(m, &sq->mousebinds, snext)
                    {
                         m->area = sq->geo;
                         m->area.y -= sq->geo.h;
                    }

               break;

          /* Rectangle */
          case 'R':
               if(sq->align != NoAlign)
                    sq->geo.y = (e->geo.h >> 1) - (sq->geo.h >> 1);

               STATUS_ALIGN(sq->align);

               draw_rect(b->dr, sq->geo, sq->color);

               if(!SLIST_EMPTY(&sq->mousebinds))
                    SLIST_FOREACH(m, &sq->mousebinds, snext)
                         m->area = sq->geo;

               break;
          }
     }
}

/* Render current statustext of an element */
void
status_render(struct element *e)
{
     struct barwin *b = SLIST_FIRST(&e->bars);

     if(!e->infobar->status)
          return;

     barwin_refresh_color(b);

     /* Use simple text instead sequence if no sequence found */
     if(SLIST_EMPTY(&e->infobar->statushead))
     {
          int l = draw_textw(e->infobar->theme, e->infobar->status);
          draw_text(b->dr, e->infobar->theme, e->geo.w - l,
                    TEXTY(e->infobar->theme, e->geo.h), b->fg, e->infobar->status);
     }
     else
          status_apply_list(e);

     barwin_refresh(b);
}

/* Parse and render statustext */
void
status_manage(struct element *e)
{
     struct status_seq *sq;
     struct mousebind *m;
     struct barwin *b = SLIST_FIRST(&e->bars);

     /*
      * Flush previous linked list of status sequences
      * and mousebind of status barwin
      */
     while(!SLIST_EMPTY(&e->infobar->statushead))
     {
          sq = SLIST_FIRST(&e->infobar->statushead);
          SLIST_REMOVE_HEAD(&e->infobar->statushead, next);
          free(sq->str);
          free(sq);
     }
     while(!SLIST_EMPTY(&b->mousebinds))
     {
          m = SLIST_FIRST(&b->mousebinds);
          SLIST_REMOVE_HEAD(&b->mousebinds, next);
          free((void*)m->cmd);
          free(m);
     }

     status_parse(e);
     status_render(e);
}

/* Syntax: "<infobar name> <status string>" */
void
uicb_status(Uicb cmd)
{
     struct infobar *ib;
     struct screen *s;
     char *p;

     if(!cmd || !(p = strchr(cmd, ' ')))
          return;

     /* Get infobar name & status */
     *p = '\0';
     ++p;

     SLIST_FOREACH(s, &W->h.screen, next)
     {
          SLIST_FOREACH(ib, &s->infobars, next)
               if(!strcmp(cmd, ib->name))
               {
                    free(ib->status);
                    ib->status = xstrdup(p);
                    infobar_elem_screen_update(s, ElemStatus);
               }
     }
}
