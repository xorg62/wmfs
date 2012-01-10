/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "status.h"
#include "barwin.h"
#include "infobar.h"
#include "util.h"

#include <string.h>
#include <ctype.h>

/* Parse arg;between;semicolon */
#define STATUS_GET_ARGS(i,  p, arg, n)                                  \
     do {                                                               \
          for(i = 0, p += 2, arg[0] = p; *p && *p != ']' && i < n; ++p) \
               if(*p == ';')                                            \
               {                                                        \
                    *p = '\0';                                          \
                    arg[++i] = ++p;                                     \
               }                                                        \
          *p = '\0';                                                    \
     } while(/* CONSTCOND */ 0);

#define STATUS_CHECK_ARGS(i, n1, n2, str, end)  \
     if(i != n1 && i != n2)                     \
     {                                          \
          str = end + 2;                        \
          continue;                             \
     }

/* Alloc & fill status_sq struct with universal options: align/position and type */
#define STATUS_FILL_SQ(sq, t, ma, shift, arg)                           \
     do {                                                               \
          sq = xcalloc(1, sizeof(struct status_seq));                   \
          sq->type = t;                                                 \
                                                                        \
          if(i == ma || !strcmp(arg[0], "right") || !strcmp(arg[0], "left")) \
               sq->align = str_to_position(arg[0]);                     \
          else                                                          \
          {                                                             \
               sq->align = NoAlign;                                     \
               sq->geo.x = ATOI(arg[0]);                                \
               sq->geo.y = ATOI(arg[1]);                                \
               shift = 1;                                               \
          }                                                             \
     } while(/* CONSTCOND */ 0);                                        \

static void
status_parse(struct infobar *ib)
{
     struct status_seq *sq, *prev = NULL;
     int i, shift = 0;
     char *dstr = xstrdup(ib->status), *sauv = dstr;
     char type, *p, *end, *arg[6] = { NULL };

     for(; *dstr; ++dstr)
     {
          /* Check if this is a sequence */
          if(*dstr != '\\')
               continue;

          p = ++dstr;

          if(!(strchr("sR", *p)) || !(end = strstr(p, "]\\")))
               continue;

          /* Then parse & list it */
          switch((type = tolower(*p)))
          {
          /*
           * Text sequence: \s[left/right;#color;text]\ OR \s[x;y;#color;text]\
           */
          case 's':
               STATUS_GET_ARGS(i, p, arg, 4);
               STATUS_CHECK_ARGS(i, 2, 3, dstr, end);
               STATUS_FILL_SQ(sq, type, 2, shift, arg);

               sq->color = color_atoh(arg[1 + shift]);
               sq->str = xstrdup(arg[2 + shift]);

               break;

         /*
          * Rectangle sequence: \s[left/right;w;h;#color]\ OR \s[x;y;w;h;#color]\
          */
          case 'r':
               STATUS_GET_ARGS(i, p, arg, 5);
               STATUS_CHECK_ARGS(i, 3, 4, dstr, end);
               STATUS_FILL_SQ(sq, type, 3, shift, arg);

               sq->geo.w = ATOI(arg[1 + shift]);
               sq->geo.h = ATOI(arg[2 + shift]);
               sq->color = color_atoh(arg[3 + shift]);

               break;
          }

          SLIST_INSERT_TAIL(&ib->statushead, sq, next, prev);

          prev = sq;
          dstr = end + 2;
          shift = 0;
     }

     free(sauv);
}

static void
status_apply_list(struct element *e)
{
     struct status_seq *sq;
     struct barwin *b = SLIST_FIRST(&e->bars);
     int left = 0, right = 0;
     int l;

     SLIST_FOREACH(sq, &e->infobar->statushead, next)
     {
          switch(sq->type)
          {
          /* Text */
          case 's':
               if(sq->align == NoAlign)
                    draw_text(b->dr, e->infobar->theme, sq->geo.x, sq->geo.y, sq->color, sq->str);
               else if(sq->align == Left)
               {
                    draw_text(b->dr, e->infobar->theme, left, TEXTY(e->infobar->theme, e->geo.h), sq->color, sq->str);
                    left += draw_textw(e->infobar->theme, sq->str);
               }
               else if(sq->align == Right)
               {
                    l = draw_textw(e->infobar->theme, sq->str);
                    draw_text(b->dr, e->infobar->theme, e->geo.w - right - l,
                              TEXTY(e->infobar->theme, e->geo.h), sq->color, sq->str);
                    right += l;
               }
               break;

          /* Rectangle */
          case 'r':
               if(sq->align != NoAlign)
                    sq->geo.y = (e->geo.h >> 1) - (sq->geo.h >> 1);

               if(sq->align == Left)
               {
                    sq->geo.x = left;
                    left += sq->geo.w;
               }
               else if(sq->align == Right)
               {
                    sq->geo.x = e->geo.w - right - sq->geo.w;
                    right += sq->geo.w;
               }

               draw_rect(b->dr, sq->geo, sq->color);
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

     SLIST_INIT(&e->infobar->statushead);

     /* Flush previous linked list of status sequences */
     while(!SLIST_EMPTY(&e->infobar->statushead))
     {
          sq = SLIST_FIRST(&e->infobar->statushead);
          SLIST_REMOVE_HEAD(&e->infobar->statushead, next);
          free(sq->str);
          free(sq);
     }

     status_parse(e->infobar);
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
