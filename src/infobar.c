/*
*      infobar.c
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

#define SPLIT_IND_S (3 + conf.border.layout)

/** Init InfoBar elements
*/
static void
infobar_init_element(InfoBar *i)
{
     int j = 0, k, n = 0;
     InfobarElem *e, *prev = NULL;
     Geo pg = { -PAD, 0, 0, 0 };

     STAILQ_INIT(&i->elemhead);

     for(; n < (int)strlen(i->elemorder); ++n)
     {
          e = zcalloc(sizeof(InfobarElem));

          /* Previous element geo */
          if(prev)
               pg = prev->geo;

          e->geo.x = pg.x + pg.width + PAD;
          e->geo.y = 0;
          e->geo.height = i->geo.height;

          switch(i->elemorder[n])
          {
               /* Tags element */
               case 't':
                    e->type      = ElemTag;
                    e->geo.width = PAD; /* Will change */

                    if(!i->tags_board)
                    {
                         i->tags_board = barwin_create(i->bar->win,
                                   e->geo.x,
                                   e->geo.y,
                                   e->geo.width,
                                   e->geo.height,
                                   conf.colors.bar, conf.colors.text,
                                   False, False, False);

                         /* Create tags window */
                         for(k = 1; k < conf.ntag[i->screen] + 1; ++k)
                         {
                              i->tags[k] = barwin_create(i->tags_board->win,
                                        j, 0,
                                        textw(tags[i->screen][k].name) + PAD,
                                        i->geo.height,
                                        conf.colors.bar, conf.colors.text,
                                        False, False, conf.border.tag);

                              j += textw(tags[i->screen][k].name) + PAD;
                              barwin_map_subwin(i->tags[k]);
                         }
                    }

                    barwin_resize(i->tags_board, (e->geo.width = j), i->geo.height);
                    barwin_map(i->tags_board);
                    barwin_map_subwin(i->tags_board);
                    break;

               /* Layout button element */
               case 'l':
                    e->type      = ElemLayout;
                    e->geo.width = (conf.layout_button_width > 0
                              ? (uint)conf.layout_button_width
                              : textw(tags[i->screen][seltag[i->screen]].layout.symbol) + PAD);

                    if(!i->layout_button)
                         i->layout_button = barwin_create(i->bar->win,
                                   e->geo.x,
                                   e->geo.y,
                                   e->geo.width,
                                   e->geo.height,
                                   conf.colors.layout_bg, conf.colors.layout_fg,
                                   False, False, conf.border.layout);

                    barwin_map(i->layout_button);

                    if(conf.border.layout)
                         barwin_map_subwin(i->layout_button);
                    break;

               /* Selbar element */
               case 's':
                    e->type = ElemSelbar;
                    i->selbar_geo = e->geo;
                    break;
          }

          STAILQ_INSERT_TAIL(&i->elemhead, e, next);

          prev = e;
          e = NULL;
     }

     return;
}

/** Init the InfoBar
*/
void
infobar_init(void)
{
     InfoBar *i;
     int s = screen_count(), sc = 0;

     if(!infobar)
          infobar = xcalloc(s, sizeof(InfoBar));

     for(; sc < s; ++sc)
     {
          i = &infobar[sc];
          i->geo.height = INFOBARH;
          i->screen = sc;
          i->elemorder = conf.bars.element_order;

          switch(tags[sc][seltag[sc]].barpos)
          {
               case IB_Hide:
                    sgeo[sc].y = spgeo[sc].y + TBARH;
                    sgeo[sc].height += INFOBARH;
                    i->geo.y = (-(i->geo.height) << 2);
                    break;
               case IB_Bottom:
                    sgeo[sc].y = TBARH;
                    i->geo.y = spgeo[sc].y + sgeo[sc].height + TBARH;
                    break;
               default:
               case IB_Top:
                    sgeo[sc].y = spgeo[sc].y + INFOBARH + TBARH;
                    i->geo.y = spgeo[sc].y;
                    break;
          }

          /* Create infobar barwindow */
          i->bar = barwin_create(ROOT,
                    sgeo[sc].x - BORDH,
                    i->geo.y,
                    sgeo[sc].width,
                    i->geo.height,
                    conf.colors.bar, conf.colors.text,
                    False, False, conf.border.bar);

          barwin_map(i->bar);
          barwin_map_subwin(i->bar);
          barwin_refresh_color(i->bar);
          barwin_refresh(i->bar);

          /* Init elements */
          infobar_init_element(i);

          /* Default statustext is set here */
          i->statustext = xstrdup("wmfs"WMFS_VERSION);

          infobar_draw(i);
     }

     return;
}

static void
infobar_arrange_element(InfoBar *i)
{
     Geo pg = { -PAD, 0, 0, 0 };
     InfobarElem *e, *pe = NULL;

     STAILQ_FOREACH(e, &i->elemhead, next)
     {
          if(pe)
               pg = pe->geo;

          e->geo.x = pg.x + pg.width + PAD;
          e->geo.y = 0;

          if(e->type != ElemSelbar)
               barwin_move((e->type == ElemTag ? i->tags_board : i->layout_button), e->geo.x, e->geo.y);

          pe = e;
     }

     return;
}

/** Draw the layout button in the InfoBar
 *\param i InfoBar pointer
 */
void
infobar_draw_layout(InfoBar *i)
{
     InfobarElem *e;
     int s = i->screen;

     /* Check if there is element in string element list */
     if(!strchr(i->elemorder, 'l'))
          return;

     STAILQ_FOREACH(e, &i->elemhead, next)
          if(e->type == ElemLayout)
               break;

     e->geo.width = (conf.layout_button_width >= 1)
          ? conf.layout_button_width
          : (int)(textw(tags[s][seltag[s]].layout.symbol) + PAD);

     barwin_resize(i->layout_button, e->geo.width, e->geo.height);
     barwin_refresh_color(i->layout_button);

     /* Split mode indicator; little rectangle at bottom-right */
     if(tags[s][seltag[s]].flags & SplitFlag)
          draw_rectangle(i->layout_button->dr,
                    e->geo.width  - SPLIT_IND_S,
                    e->geo.height - SPLIT_IND_S,
                    SPLIT_IND_S, SPLIT_IND_S,
                    getcolor(i->layout_button->fg));

     if(tags[s][seltag[s]].layout.symbol)
          barwin_draw_text(i->layout_button, (PAD >> 1), FHINFOBAR, tags[s][seltag[s]].layout.symbol);

     return;
}

/** Draw Infobar barwin (selbar / statustext)
  *\param i Infobar pointer
 */
void
_infobar_draw(InfoBar *i)
{
     barwin_refresh_color(i->bar);

     infobar_draw_selbar(i);
     statustext_handle(i);

     barwin_refresh(i->bar);

     infobar_arrange_element(i);

     return;
}

/** Draw the InfoBar
 *\param i InfoBar pointer
*/
void
infobar_draw(InfoBar *i)
{
     infobar_draw_taglist(i);
     infobar_draw_layout(i);

     _infobar_draw(i);

     return;
}

/** Draw Selbar (selected client title bar in infobar
 *\param sc Screen Number
 */
void
infobar_draw_selbar(InfoBar *i)
{
     InfobarElem *e;
     char *str = NULL;
     int sc = i->screen;
     bool f = False;

     if(!conf.bars.selbar)
          return;

     if(!strchr(i->elemorder, 's'))
          return;

     if(!sel || (sel && sel->screen != sc))
          return;

     STAILQ_FOREACH(e, &i->elemhead, next)
          if(e->type == ElemSelbar)
               break;

     str = sel->title;

     /* Truncate string if too long */
     if(conf.selbar.maxlength >= 0 && sel && sel->title)
     {
          str = NULL;
          str = xcalloc(conf.selbar.maxlength + 4, sizeof(char));
          strncpy(str, sel->title, conf.selbar.maxlength);

          if(strlen(sel->title) > (size_t)conf.selbar.maxlength)
               strcat(str, "...");

          f = True;
     }

     XSetForeground(dpy, gc, conf.selbar.bg);
     XFillRectangle(dpy, i->bar->dr, gc,
               e->geo.x,
               e->geo.y,
               (e->geo.width = textw(str) + PAD),
               e->geo.height);
     draw_text(i->bar->dr, e->geo.x, FHINFOBAR, conf.selbar.fg, str);

     if(f)
          free(str);

     i->selbar_geo = e->geo;

     return;
}

/** Draw the taglist in the InfoBar
 *\param i InfoBar pointer
*/
void
infobar_draw_taglist(InfoBar *i)
{
     InfobarElem *e;
     Client *c;
     int j = 1, x = 0, sc = i->screen;
     uint occupied = 0;

     if(!strchr(i->elemorder, 't'))
          return;

     STAILQ_FOREACH(e, &i->elemhead, next)
          if(e->type == ElemTag)
               break;

     SLIST_FOREACH(c, &clients, next)
          if(c->screen == sc)
               occupied |= TagFlag(c->tag);

     for(; j < conf.ntag[sc] + 1; ++j)
     {
          /* Autohide tag feature */
          if(conf.tagautohide)
          {
               if(!(occupied & TagFlag(j)) && j != seltag[sc])
               {
                    barwin_unmap(i->tags[j]);
                    continue;
               }

               barwin_map(i->tags[j]);
               barwin_move(i->tags[j], x, 0);
               barwin_resize(i->tags_board, (e->geo.width = (x += i->tags[j]->geo.width)), e->geo.height);
          }

          if(tags[sc][j].flags & TagUrgentFlag)
          {
               i->tags[j]->bg = conf.colors.tagurbg;
               i->tags[j]->fg = conf.colors.tagurfg;
          }
          else if(j == seltag[sc] || tags[sc][seltag[sc]].tagad & TagFlag(j))
          {
               i->tags[j]->bg = conf.colors.tagselbg;
               i->tags[j]->fg = conf.colors.tagselfg;
          }
          else
          {
               i->tags[j]->bg = ((occupied & TagFlag(j)) ? conf.colors.tag_occupied_bg : conf.colors.bar);
               i->tags[j]->fg = ((occupied & TagFlag(j)) ? conf.colors.tag_occupied_fg : conf.colors.text);
          }

          barwin_color_set(i->tags[j], i->tags[j]->bg, i->tags[j]->fg);
          barwin_refresh_color(i->tags[j]);

          if(tags[sc][j].name)
               barwin_draw_text(i->tags[j], (PAD >> 1), FHINFOBAR, tags[sc][j].name);
     }

     return;
}

/** Update taglist geo
  *\param i InfoBar pointer
*/
void
infobar_update_taglist(InfoBar *i)
{
     int t, j, sc = i->screen;

     for(t = 1, j = 0; t < conf.ntag[sc] + 1; ++t)
     {
          /* If the tag t does not exist yet (graphically) or need full update */
          if(!i->tags[t] || i->need_update)
          {
               i->tags[t] = barwin_create(i->tags_board->win, j, 0,
                         textw(tags[sc][t].name) + PAD,
                         i->geo.height,
                         conf.colors.bar, conf.colors.text, False, False, conf.border.tag);

               barwin_map(i->tags[t]);
               barwin_map_subwin(i->tags[t]);
               barwin_resize(i->tags_board, (j += textw(tags[sc][t].name) + PAD), i->geo.height);

               continue;
          }

          barwin_move(i->tags[t], j, 0);
          barwin_resize(i->tags[t], textw(tags[sc][t].name) + PAD, i->geo.height);
          barwin_resize(i->tags_board, (j += textw(tags[sc][t].name) + PAD), i->geo.height);
     }

     i->need_update = False;

     return;
}

/** Destroy the InfoBar
*/
void
infobar_destroy(void)
{
     InfobarElem *e;
     int sc, i;

     for(sc = 0; sc < screen_count(); ++sc)
     {
          barwin_delete(infobar[sc].layout_button);
          barwin_delete_subwin(infobar[sc].layout_button);

          for(i = 1; i < conf.ntag[sc] + 1; ++i)
          {
               barwin_delete_subwin(infobar[sc].tags[i]);
               barwin_delete(infobar[sc].tags[i]);
          }

          barwin_delete_subwin(infobar[sc].tags_board);
          barwin_delete(infobar[sc].tags_board);
          barwin_delete_subwin(infobar[sc].bar);
          barwin_delete(infobar[sc].bar);

          /* Free elements */
          while(!STAILQ_EMPTY(&infobar[sc].elemhead))
          {
               e = STAILQ_FIRST(&infobar[sc].elemhead);
               STAILQ_REMOVE_HEAD(&infobar[sc].elemhead, next);
               free(e);
          }

     }

     return;
}

/* Set the infobar position
 * \param pos Position of the bar
 */
void
infobar_set_position(int pos)
{
     screen_get_sel();

     switch(pos)
     {
          case IB_Hide:
               sgeo[selscreen].y = spgeo[selscreen].y + TBARH;
               sgeo[selscreen].height = spgeo[selscreen].height - TBARH;
               infobar[selscreen].geo.y = (-(infobar[selscreen].geo.height) << 1);
               break;
          case IB_Bottom:
               sgeo[selscreen].y = spgeo[selscreen].y + TBARH;
               sgeo[selscreen].height = spgeo[selscreen].height - INFOBARH - TBARH;
               infobar[selscreen].geo.y = spgeo[selscreen].y + sgeo[selscreen].height + TBARH;
               break;
          default:
          case IB_Top:
               sgeo[selscreen].y = spgeo[selscreen].y + INFOBARH + TBARH;
               sgeo[selscreen].height = spgeo[selscreen].height - INFOBARH - TBARH;
               infobar[selscreen].geo.y = spgeo[selscreen].y;
               break;
     }

     tags[selscreen][seltag[selscreen]].barpos = pos;

     barwin_move(infobar[selscreen].bar, sgeo[selscreen].x - BORDH, infobar[selscreen].geo.y);
     arrange(selscreen, True);

     return;
}

/** Toggle the infobar position
 * \param cmd uicb_t type unused
*/
void
uicb_infobar_togglepos(uicb_t cmd)
{
     (void)cmd;
     screen_get_sel();

     infobar_set_position((tags[selscreen][seltag[selscreen]].barpos
                           = (tags[selscreen][seltag[selscreen]].barpos < 2)
                           ? tags[selscreen][seltag[selscreen]].barpos + 1
                           : 0));

     return;
}

/** Toggle hide/display infobar
 * \param cmd uicb_t type unused
*/
void
uicb_infobar_toggledisplay(uicb_t cmd)
{
     (void)cmd;
     screen_get_sel();
     int new_pos;

     new_pos = (tags[selscreen][seltag[selscreen]].barpos
               ? 0 : (tags[selscreen][seltag[selscreen]].prev_barpos
                    ? tags[selscreen][seltag[selscreen]].prev_barpos : 2));

     tags[selscreen][seltag[selscreen]].prev_barpos = tags[selscreen][seltag[selscreen]].barpos;
     tags[selscreen][seltag[selscreen]].barpos = new_pos;

     infobar_set_position(new_pos);

     return;
}

/** Toggle the tag_autohide mode
 * \param cmd uicb_t type unused
*/
void
uicb_toggle_tagautohide(uicb_t cmd)
{
     int i, x;
     (void)cmd;

     screen_get_sel();
     conf.tagautohide = !conf.tagautohide;

     if(!conf.tagautohide)
     {
          for(i = 1, x = 0; i < conf.ntag[selscreen] + 1; ++i)
          {
               barwin_map(infobar[selscreen].tags[i]);
               barwin_move(infobar[selscreen].tags[i], x, 0);
               x += infobar[selscreen].tags[i]->geo.width;
          }

          barwin_resize(infobar[selscreen].tags_board, x, infobar[selscreen].geo.height);
     }

     infobar_draw(&infobar[selscreen]);

     return;
}
