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
     SLIST_INIT(&ctx.gcache);

     return ctx;
}

static void
status_gcache_free(struct status_ctx *ctx)
{
     struct status_gcache *gc;

     while(!SLIST_EMPTY(&ctx->gcache))
     {
          gc = SLIST_FIRST(&ctx->gcache);
          SLIST_REMOVE_HEAD(&ctx->gcache, next);
          free(gc->datas);
          free(gc->name);
          free(gc);
     }
}

void
status_free_ctx(struct status_ctx *ctx)
{
     free(ctx->status);
     status_flush_list(ctx);
     status_gcache_free(ctx);
}

static void
status_graph_draw(struct status_ctx *ctx, struct status_seq *sq, struct status_gcache *gc)
{
     int i, j, y;
     float c;
     int ys = sq->geo.y + sq->geo.h - 1;

     XSetForeground(W->dpy, W->gc, sq->bg);

     for(i = sq->geo.x + sq->geo.w - 1, j = gc->ndata - 1;
         j >= 0 && i >= sq->geo.x;
         --j, --i)
     {
          /* You divided by zero didn't you? */
          if(gc->datas[j])
          {
               c = (float)sq->data[2] / (float)gc->datas[j];
               y = ys - (sq->geo.h / (c > 1 ? c : 1)) + 1;
               draw_line(ctx->barwin->dr, i, y, i, ys);
          }
     }
}

static void
status_graph_process(struct status_ctx *ctx, struct status_seq *sq, char *name)
{
     int j;
     struct status_gcache *gc;

     /* Graph already exist and have a cache */
     SLIST_FOREACH(gc, &ctx->gcache, next)
          if(!strcmp(name, gc->name))
          {
               /* shift buffer to remove unused old value */
               if(gc->ndata > (sq->geo.w << 1))
                    for(gc->ndata /= 2, j = 0;
                        j < gc->ndata;
                        gc->datas[j] = gc->datas[j + gc->ndata], ++j);

               gc->datas[gc->ndata++] = sq->data[1];
               status_graph_draw(ctx, sq, gc);
               return;
          }

     if(sq->data[1] > sq->data[2])
          sq->data[1] = sq->data[2];

     /* No? Make a cache for it */
     gc = xcalloc(1, sizeof(struct status_gcache));
     gc->name = xstrdup(name);
     gc->ndata = 1;
     gc->datas = xcalloc(sq->geo.w << 2, sizeof(int));
     gc->datas[0] = sq->data[1];

     SLIST_INSERT_HEAD(&ctx->gcache, gc, next);

     status_graph_draw(ctx, sq, gc);
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
     int i, tmp, shift = 0;
     char *dstr = xstrdup(ctx->status), *sauv = dstr;
     char type, *p, *pp, *end, *arg[10] = { NULL };

     for(; *dstr; ++dstr)
     {
          /* Check if this is a sequence */
          if(*dstr != '^' && *dstr != '\\')
               continue;

          p = ++dstr;

          /* Search for correct end of sequence (] without \ behind) */
          if((end = strchr(p, ']')))
               while(*(end - 1) == '\\')
                    end = strchr(end + 1, ']');

          if(!(strchr("sRpPig", *p)) || !end)
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

               sq->fg = fgcolor_atoh(arg[1 + shift]);
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
               sq->fg = fgcolor_atoh(arg[3 + shift]);

               break;

          /*
           * Progress bar sequence: \p[left/right;w;h;bord;val;valmax;bg;fg] OR x;y
           * Position bar sequence: \P[left/right;w;h;tickbord;val;valmax;bg;fg] OR x;y
           */
          case 'p':
          case 'P':
               i = parse_args(p + 2, ';', ']', 9, arg);
               STATUS_CHECK_ARGS(i, 7, 8, dstr, end);
               sq = status_new_seq(type, i, 7, arg, &shift);

               sq->geo.w = ATOI(arg[1 + shift]);
               sq->geo.h = ATOI(arg[2 + shift]);

               sq->data[0] = ATOI(arg[3 + shift]);                     /* Border */
               sq->data[1] = ((tmp = ATOI(arg[4 + shift])) ? tmp : 1); /* Value */
               sq->data[2] = ATOI(arg[5 + shift]);                     /* Value Max */

               sq->fg = fgcolor_atoh(arg[6 + shift]);
               sq->bg = bgcolor_atoh(arg[7 + shift]);

               break;

          /*
           * Graph sequence: \g[left/right;w;h;val;valmax;bg;fg;name] OR x;y
           */
          case 'g':
               i = parse_args(p + 2, ';', ']', 9, arg);
               STATUS_CHECK_ARGS(i, 7, 8, dstr, end);
               sq = status_new_seq(type, i, 7, arg, &shift);

               sq->geo.w = ATOI(arg[1 + shift]);
               sq->geo.h = ATOI(arg[2 + shift]);

               sq->data[1] = ATOI(arg[3 + shift]); /* Value */
               sq->data[2] = ATOI(arg[4 + shift]); /* Value Max */

               sq->fg = fgcolor_atoh(arg[5 + shift]);
               sq->bg = bgcolor_atoh(arg[6 + shift]);

               sq->str = xstrdup(arg[7 + shift]);

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

          if(sq->align == Right)
              SLIST_INSERT_HEAD(&ctx->statushead, sq, next);
          else
              SLIST_INSERT_TAIL(&ctx->statushead, sq, next, prev);

          /*
           * Optional mousebind sequence(s) \<seq>[](button;func;cmd)
           * Parse it while there is a mousebind sequence.
           */
          dstr = end + 1;

          do
               dstr = status_parse_mouse(sq, dstr);
          while(*dstr == '(');

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

#define STORE_MOUSEBIND()                                       \
     if(!SLIST_EMPTY(&sq->mousebinds))                          \
          SLIST_FOREACH(m, &sq->mousebinds, snext)              \
               m->area = sq->geo;

#define NOALIGN_Y()                                                     \
     if(sq->align != NoAlign)                                           \
          sq->geo.y = (ctx->barwin->geo.h >> 1) - (sq->geo.h >> 1);
static void
status_apply_list(struct status_ctx *ctx)
{
     struct status_seq *sq;
     struct mousebind *m;
     struct geo g;
     int left = 0, right = 0, w, h;

     SLIST_FOREACH(sq, &ctx->statushead, next)
     {
          switch(sq->type)
          {
          /* Text */
          case 's':
               sq->geo.w = draw_textw(ctx->theme, sq->str);
#ifdef HAVE_XFT
               sq->geo.h = ctx->theme->font->height;
#else
               sq->geo.h = ctx->theme->font.height;
#endif

               if(sq->align != NoAlign)
                    sq->geo.y = TEXTY(ctx->theme, ctx->barwin->geo.h);

               STATUS_ALIGN(sq->align);

#ifdef HAVE_XFT
               draw_text(ctx->barwin->xftdraw, ctx->theme, sq->geo.x, sq->geo.y, sq->fg, sq->str);
#else
               draw_text(ctx->barwin->dr, ctx->theme, sq->geo.x, sq->geo.y, sq->fg, sq->str);
#endif /* HAVE_XFT */

               if(!SLIST_EMPTY(&sq->mousebinds))
                    SLIST_FOREACH(m, &sq->mousebinds, snext)
                    {
                         m->area = sq->geo;
                         m->area.y -= sq->geo.h;
                    }

               break;

          /* Rectangle */
          case 'R':
               NOALIGN_Y();
               STATUS_ALIGN(sq->align);

               draw_rect(ctx->barwin->dr, &sq->geo, sq->bg);

               STORE_MOUSEBIND();

               break;

          /* Progress */
          case 'p':
               NOALIGN_Y();
               STATUS_ALIGN(sq->align);

               draw_rect(ctx->barwin->dr, &sq->geo, sq->bg);

               /* Progress bar geo */
               g.x = sq->geo.x + sq->data[0];
               g.y = sq->geo.y + sq->data[0];
               g.w = sq->geo.w - sq->data[0] - sq->data[0];
               g.h = sq->geo.h - sq->data[0] - sq->data[0];

               if(sq->data[1] > sq->data[2])
                    sq->data[1] = sq->data[2];

               if(sq->geo.w > sq->geo.h)
                    g.w /= ((float)sq->data[2] / (float)sq->data[1]);
               else
               {
                    g.y += g.h;
                    g.h /= ((float)sq->data[2] / (float)sq->data[1]);
                    g.y -= g.h;
               }

               draw_rect(ctx->barwin->dr, &g, sq->bg);

               STORE_MOUSEBIND();

               break;

          /* Position */
          case 'P':
               NOALIGN_Y();
               STATUS_ALIGN(sq->align);

               draw_rect(ctx->barwin->dr, &sq->geo, sq->bg);

               if(sq->data[1] > sq->data[2])
                    sq->data[1] = sq->data[2];

               g.x = sq->geo.x + ((sq->geo.w - sq->data[0]) / ((float)sq->data[2] / (float)sq->data[1]));
               g.y = sq->geo.y;
               g.w = sq->data[0];
               g.h = sq->geo.h;

               draw_rect(ctx->barwin->dr, &g, sq->bg);

               STORE_MOUSEBIND();

               break;

          /* Graph */
          case 'g':
               NOALIGN_Y();
               STATUS_ALIGN(sq->align);

               draw_rect(ctx->barwin->dr, &sq->geo, sq->bg);

               status_graph_process(ctx, sq, sq->str);

               STORE_MOUSEBIND();

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
               STORE_MOUSEBIND();

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

     if(!(ctx->flags & STATUS_BLOCK_REFRESH))
          barwin_refresh_color(ctx->barwin);

     /* Use simple text instead sequence if no sequence found */
     if(SLIST_EMPTY(&ctx->statushead))
     {
          int l = draw_textw(ctx->theme, ctx->status);
#ifdef HAVE_XFT
          draw_text(ctx->barwin->xftdraw, ctx->theme, ctx->barwin->geo.w - l,
                    TEXTY(ctx->theme, ctx->barwin->geo.h), ctx->barwin->fg, ctx->status);
#else
          draw_text(ctx->barwin->dr, ctx->theme, ctx->barwin->geo.w - l,
                    TEXTY(ctx->theme, ctx->barwin->geo.h), ctx->barwin->fg, ctx->status);
#endif /* HAVE_XFT */
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

     SLIST_INIT(&ctx->statushead);
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

void
status_flush_surface(void)
{
     struct barwin *b;

     while(!SLIST_EMPTY(&W->h.vbarwin))
     {
          b = SLIST_FIRST(&W->h.vbarwin);
          SLIST_REMOVE_HEAD(&W->h.vbarwin, vnext);
          barwin_remove(b);
     }
}

static void
status_surface(int x, int y, int w, int h, BgColor bg, char *status)
{
     struct barwin *b;
     struct screen *s;
     struct status_ctx ctx;
     int d;
     Window rw;

     if(!status)
          return;

     if(x + y < 0)
          XQueryPointer(W->dpy, W->root, &rw, &rw, &x, &y, &d, &d, (unsigned int *)&d);

     s = screen_gb_geo(x, y);

     if(x + w > s->geo.x + s->geo.w)
          x -= w;
     if(y + h > s->geo.y + s->geo.h)
          y -= h;

     b = barwin_new(W->root, x, y, w, h, W->ctheme->bars.fg, bg, false);
     barwin_map(b);

     /* Use client theme */
     ctx = status_new_ctx(b, W->ctheme);
     ctx.status = xstrdup(status);

     SLIST_INSERT_HEAD(&W->h.vbarwin, b, vnext);

     status_manage(&ctx);
     status_free_ctx(&ctx);
}

void
uicb_status_surface(Uicb cmd)
{
     char *p, *ccmd = xstrdup(cmd);
     int s, w, h, x = -1, y = -1;
     BgColor bg;

     if(!ccmd || !(p = strchr(ccmd, ' ')))
          return;

     *p = '\0';
     ++p;

     if(!(((s = sscanf(ccmd, "%d,%d,#%x", &w, &h, &bg)) == 3)
        || (s = sscanf(ccmd, "%d,%d,%d,%d,#%x", &x, &y, &w, &h, &bg)) == 5))
     {
          free(ccmd);
          return;
     }

     status_surface(x, y, w, h, bg, p);

     free(ccmd);
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
