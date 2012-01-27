/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "status.h"
#include "barwin.h"
#include "config.h"
#include "infobar.h"
#include "util.h"
#include "draw.h"

#include <string.h>

struct status_seq*
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

struct status_ctx
status_new_ctx(struct barwin *b, struct theme *t)
{
     struct status_ctx ctx = { .barwin = b, .theme = t };

     SLIST_INIT(&ctx.statushead);

     return ctx;
}

void
status_free_ctx(struct status_ctx *ctx)
{
     free(ctx->status);

     if(ctx->barwin)
          SLIST_INIT(&ctx->barwin->statusmousebinds);

     status_flush_list(ctx);
}

/* Parse mousebind sequence next normal sequence: \<seq>[](button;func;cmd) */
static char*
status_parse_mouse(struct status_seq *sq, char *str)
{
     struct mousebind *m;
     char *end, *arg[3] = { NULL };
     int i;

     if(*str != '(' || !(end = strchr(str, ')')))
          return str;

     i = parse_args(++str, ';', ')', 3, arg);

     m = xcalloc(1, sizeof(struct mousebind));

     m->use_area = true;
     m->button   = ATOI(arg[0]);
     m->func     = uicb_name_func(arg[1]);
     m->cmd      = (i > 1 ? xstrdup(arg[2]) : NULL);

     SLIST_INSERT_HEAD(&sq->mousebinds, m, snext);

     return end + 1;
}

#define STATUS_CHECK_ARGS(i, n1, n2, str, end)  \
     if(i != n1 && i != n2)                     \
     {                                          \
          str = end + 2;                        \
          continue;                             \
     }
void
status_parse(struct status_ctx *ctx)
{
     struct status_seq *sq, *prev = NULL;
     int i, shift = 0;
     char *dstr = xstrdup(ctx->status), *sauv = dstr;
     char type, *p, *pp, *end, *arg[6] = { NULL };

     for(; *dstr; ++dstr)
     {
          /* Check if this is a sequence */
          if(*dstr != '^' && *dstr != '\\')
               continue;

          p = ++dstr;

          /* Search end of sequence (] without \ behind) */
          for(end = strchr(p, ']'); *(end - 1) == '\\';)
               end = strchr(end + 1, ']');

          if(!(strchr("sRi", *p)) || !end )
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

               /* Remove \ from string */
               for(pp = sq->str; (pp = strchr(sq->str, '\\'));)
                    memmove(pp, pp + 1, strlen(pp));

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

          /*
           * Image sequence: \i[left/right;w;h;/path/img] OR \i[x;y;w;h;/path/img]
           */
#ifdef HAVE_IMLIB2
          case 'i':
               i = parse_args(p + 2, ';', ']', 5, arg);
               STATUS_CHECK_ARGS(i, 3, 4, dstr, end);
               sq = status_new_seq(type, i, 3, arg, &shift);

               sq->geo.w = ATOI(arg[1 + shift]);
               sq->geo.h = ATOI(arg[2 + shift]);
               sq->str   = xstrdup(arg[3 + shift]);

               break;
#endif /* HAVE_IMLIB2 */
          }

          if (sq->align == Right)
              SLIST_INSERT_HEAD(&ctx->statushead, sq, next);
          else
              SLIST_INSERT_TAIL(&ctx->statushead, sq, next, prev);

          /*
           * Optional mousebind sequence(s) \<seq>[](button;func;cmd)
           * Parse it while there is a mousebind sequence.
           */
          dstr = end + 1;
          while((*(dstr = status_parse_mouse(sq, dstr)) == '('));
          --dstr;

          prev = sq;
     }

     free(sauv);
}

#define STATUS_ALIGN(align)                                       \
     if(align == Left)                                            \
     {                                                            \
          sq->geo.x = left;                                       \
          left += sq->geo.w;                                      \
     }                                                            \
     else if(align == Right)                                      \
     {                                                            \
          sq->geo.x = ctx->barwin->geo.w - right - sq->geo.w;     \
          right += sq->geo.w;                                     \
     }
static void
status_apply_list(struct status_ctx *ctx)
{
     struct status_seq *sq;
     struct mousebind *m;
     int left = 0, right = 0, w, h;

     SLIST_FOREACH(sq, &ctx->statushead, next)
     {
          switch(sq->type)
          {
          /* Text */
          case 's':
               sq->geo.w = draw_textw(ctx->theme, sq->str);
               sq->geo.h = ctx->theme->font.height;

               if(sq->align != NoAlign)
                    sq->geo.y = TEXTY(ctx->theme, ctx->barwin->geo.h);

               STATUS_ALIGN(sq->align);

               draw_text(ctx->barwin->dr, ctx->theme, sq->geo.x, sq->geo.y, sq->color, sq->str);

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
                    sq->geo.y = (ctx->barwin->geo.h >> 1) - (sq->geo.h >> 1);

               STATUS_ALIGN(sq->align);

               draw_rect(ctx->barwin->dr, &sq->geo, sq->color);

               if(!SLIST_EMPTY(&sq->mousebinds))
                    SLIST_FOREACH(m, &sq->mousebinds, snext)
                         m->area = sq->geo;

               break;

          /* Image */
#ifdef HAVE_IMLIB2
          case 'i':
               draw_image_load(sq->str, &w, &h);

               if(sq->geo.w <= 0)
                    sq->geo.w = w;
               if(sq->geo.h <= 0)
                    sq->geo.h = h;

               if(sq->align != NoAlign)
                    sq->geo.y = (ctx->barwin->geo.h >> 1) - (sq->geo.h >> 1);

               STATUS_ALIGN(sq->align);

               draw_image(ctx->barwin->dr, &sq->geo);

               if(!SLIST_EMPTY(&sq->mousebinds))
                    SLIST_FOREACH(m, &sq->mousebinds, snext)
                         m->area = sq->geo;

               break;
#endif /* HAVE_IMLIB2 */

          }
     }
}

/* Render current statustext of an element */
void
status_render(struct status_ctx *ctx)
{
     if(!ctx->status)
          return;

     barwin_refresh_color(ctx->barwin);

     /* Use simple text instead sequence if no sequence found */
     if(SLIST_EMPTY(&ctx->statushead))
     {
          int l = draw_textw(ctx->theme, ctx->status);
          draw_text(ctx->barwin->dr, ctx->theme, ctx->barwin->geo.w - l,
                    TEXTY(ctx->theme, ctx->barwin->geo.h), ctx->barwin->fg, ctx->status);
     }
     else
          status_apply_list(ctx);

     barwin_refresh(ctx->barwin);
}

void
status_flush_list(struct status_ctx *ctx)
{
     struct status_seq *sq;
     struct mousebind *m;

     /* Flush previous linked list of status sequences */
     while(!SLIST_EMPTY(&ctx->statushead))
     {
          sq = SLIST_FIRST(&ctx->statushead);
          SLIST_REMOVE_HEAD(&ctx->statushead, next);

          while(!SLIST_EMPTY(&sq->mousebinds))
          {
               m = SLIST_FIRST(&sq->mousebinds);
               SLIST_REMOVE_HEAD(&sq->mousebinds, snext);
               free((void*)m->cmd);
               free(m);
          }

          free(sq->str);
          free(sq);
     }
}

void
status_copy_mousebind(struct status_ctx *ctx)
{
     struct mousebind *m;
     struct status_seq *sq;

     if(!ctx->barwin)
          return;

     /* Flush barwin head of status mousebinds */
     SLIST_INIT(&ctx->barwin->statusmousebinds);

     SLIST_FOREACH(sq, &ctx->statushead, next)
     {
          SLIST_FOREACH(m, &sq->mousebinds, snext)
               SLIST_INSERT_HEAD(&ctx->barwin->statusmousebinds, m, next);
     }
}

/* Parse and render statustext */
void
status_manage(struct status_ctx *ctx)
{
     if(!ctx->status)
          return;

     ctx->update = false;

     status_flush_list(ctx);
     status_parse(ctx);
     status_render(ctx);
     status_copy_mousebind(ctx);
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
                    free(ib->statusctx.status);
                    ib->statusctx.status = xstrdup(p);
                    ib->statusctx.update = true;
                    infobar_elem_screen_update(s, ElemStatus);
               }
     }
}
