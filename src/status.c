/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "status.h"
#include "barwin.h"
#include "infobar.h"
#include "util.h"

#include <string.h>

/* Parse arg;between;semicolon */
#define STATUS_GET_ARGS(i, inc, p, arg, n)                                \
     do {                                                                 \
          for(i = 0, p += inc, arg[0] = p; *p && *p != ']' && i < n; ++p) \
               if(*p == ';')                                              \
               {                                                          \
                    *p = '\0';                                            \
                    arg[++i] = ++p;                                       \
               }                                                          \
          *p = '\0';                                                      \
     } while(/* CONSTCOND */ 0);

#define STATUS_CHECK_END(str, end, p)           \
     if(!(end = strstr(p, "]\\")))              \
     {                                          \
          str = end + 2;                        \
          continue;                             \
     }

#define STATUS_CHECK_ARGS(i, n1, n2, str, end)  \
     if(i != n1 && i != n2)                     \
     {                                          \
          str = end + 2;                        \
          continue;                             \
     }

/* Alloc & fill status_sq struct with universal options: align/position and type */
#define STATUS_FILL_SQ(sq, t, shift, arg)                               \
     do {                                                               \
          sq = xcalloc(1, sizeof(struct status_seq));                   \
          sq->type = t;                                                 \
                                                                        \
          if(!strcmp(arg[0], "right") || !strcmp(arg[0], "left"))       \
               sq->align = str_to_position(arg[0]);                     \
          else                                                          \
          {                                                             \
               sq->align = NoAlign;                                     \
               sq->geo.x = ATOI(arg[0]);                                \
               sq->geo.y = ATOI(arg[1]);                                \
               shift = 1;                                               \
          }                                                             \
     } while(/* CONSTCOND */ 0);                                        \


/** Check text blocks in str and list it
 * --> \s[left/right;#color;text]\ OR \s[x;y;#color;text]\
 *\param ib Infobar pointer
 *\param str String
 */
static void
status_text(struct infobar *ib, char *str)
{
     struct status_seq *sq;
     int i;
     char *dstr = xstrdup(str), *sauv = dstr;
     char *p, *end;

     while((p = strstr(dstr, "\\s[")))
     {
          char *arg[5] = { NULL };
          int shift = 0;

          STATUS_CHECK_END(dstr, end, p);
          STATUS_GET_ARGS(i, 3, p, arg, 4);
          STATUS_CHECK_ARGS(i, 2, 3, dstr, end);
          STATUS_FILL_SQ(sq, 's', shift, arg);

          /* string sequence options */
          sq->color = color_atoh(arg[1 + shift]);
          sq->str = xstrdup(arg[2 + shift]);

          SLIST_INSERT_HEAD(&ib->statushead, sq, next);

          dstr = end + 2;
     }

     free(sauv);
}

/** Check rectangle blocks in str and list it
 * --> \s[left/right;width;height;#color]\ OR \s[x;y;width;height;#color]\
 *\param ib Infobar pointer
 *\param str String
 */
static void
status_rect(struct infobar *ib, char *str)
{
     struct status_seq *sq;
     int i;
     char *dstr = xstrdup(str), *sauv = dstr;
     char *p, *end;

     while((p = strstr(dstr, "\r[")))
     {
          char *arg[6] = { NULL };
          int shift = 0;

          STATUS_CHECK_END(dstr, end, p);
          STATUS_GET_ARGS(i, 2, p, arg, 5);
          STATUS_CHECK_ARGS(i, 3, 4, dstr, end);
          STATUS_FILL_SQ(sq, 'r', shift, arg);

          /* rectangle sequence options */
          sq->geo.w = ATOI(arg[1 + shift]);
          sq->geo.h = ATOI(arg[2 + shift]);
          sq->color = color_atoh(arg[3 + shift]);

          SLIST_INSERT_HEAD(&ib->statushead, sq, next);

          dstr = end + 2;
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

void
status_manage(struct element *e)
{
     struct status_seq *sq;
     struct barwin *b = SLIST_FIRST(&e->bars);

     SLIST_INIT(&e->infobar->statushead);

     /* Flush previous linked list of status sequences */
     while(!SLIST_EMPTY(&e->infobar->statushead))
     {
          sq = SLIST_FIRST(&e->infobar->statushead);
          SLIST_REMOVE_HEAD(&e->infobar->statushead, next);
          free(sq->str);
          free(sq);
     }

     if(!e->infobar->status)
          return;

     barwin_refresh_color(b);

     status_text(e->infobar, e->infobar->status);
     status_rect(e->infobar, e->infobar->status);

     /* DEBUG
        SLIST_FOREACH(sq, &e->infobar->statushead, next)
        printf("-> %c (%d;%d)x(%d;%d) (%d) (#%x) (%s)\n",
        sq->type, sq->geo.x, sq->geo.y, sq->geo.w, sq->geo.h, sq->align, sq->color, sq->str);
     */

     status_apply_list(e);

     /*
       l = draw_textw(e->infobar->theme, e->infobar->status);
       draw_text(b->dr, e->infobar->theme, e->geo.w - l,
       TEXTY(e->infobar->theme, e->geo.h), b->fg, e->infobar->status);
     */

     barwin_refresh(b);

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
