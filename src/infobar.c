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

/** Init the InfoBar
*/
void
infobar_init(void)
{
     InfoBar *ib;
     int s, sc, i, j = 0;

     s = screen_count();

     if(!infobar)
          infobar = xcalloc(s, sizeof(InfoBar));

     for(sc = 0; sc < s; ++sc)
     {
          j = 0;
          ib = &infobar[sc];
          ib->geo.height = INFOBARH;
          ib->screen = sc;

          switch(tags[sc][seltag[sc]].barpos)
          {
          case IB_Hide:
               sgeo[sc].y = spgeo[sc].y + TBARH;
               sgeo[sc].height += INFOBARH;
               ib->geo.y = (-(ib->geo.height) << 2);
               break;
          case IB_Bottom:
               sgeo[sc].y = TBARH;
               ib->geo.y = spgeo[sc].y + sgeo[sc].height + TBARH;
               break;
          default:
          case IB_Top:
               sgeo[sc].y = spgeo[sc].y + INFOBARH + TBARH;
               ib->geo.y = spgeo[sc].y;
               break;
          }

          /* Create infobar barwindow */
          ib->bar = barwin_create(ROOT, sgeo[sc].x - BORDH, ib->geo.y,
                                          sgeo[sc].width, ib->geo.height,
                                          conf.colors.bar, conf.colors.text, False, False, conf.border.bar);

          ib->tags_board = barwin_create(ib->bar->win,
                    ((conf.layout_placement) ? textw(tags[sc][seltag[sc]].layout.symbol) + PAD * 1.5: 0), 0,
                    textw(tags[sc][0].name) + PAD, /* Base size, will change */
                    ib->geo.height,
                    conf.colors.bar, conf.colors.text, False, False, False);

          /* Create tags window */
          for(i = 1; i < conf.ntag[sc] + 1; ++i)
          {
               ib->tags[i] = barwin_create(ib->tags_board->win, j, 0,
                                                   textw(tags[sc][i].name) + PAD,
                                                   ib->geo.height,
                                                   conf.colors.bar, conf.colors.text, False, False, conf.border.tag);

               j += textw(tags[sc][i].name) + PAD;

               barwin_resize(ib->tags_board, j, ib->geo.height);
               barwin_map_subwin(ib->tags[i]);
          }

          /* Create layout switch barwindow */
          ib->layout_button = barwin_create(ib->bar->win,
                    ((conf.layout_placement) ? 0 : (j + (PAD >> 1))), 0,
                    ((conf.layout_button_width > 0) ? (uint)conf.layout_button_width : (textw(tags[sc][seltag[sc]].layout.symbol) + PAD)),
                    ib->geo.height,
                    conf.colors.layout_bg, conf.colors.layout_fg,
                    False, False, conf.border.layout);

          /* Selbar */
          if(conf.bars.selbar)
               ib->selbar = barwin_create(ib->bar->win,
                         ((conf.layout_placement)
                          ? (j + (PAD >> 1))
                          : ib->layout_button->geo.x + ib->layout_button->geo.width + (PAD >> 1)), 1,
                         (sel) ? textw(sel->title) + PAD : 1,
                         ib->geo.height - 2,
                         conf.selbar.bg, conf.selbar.fg, False, False, False);

          /* Map/Refresh all */
          barwin_map(ib->bar);
          barwin_map_subwin(ib->bar);

          barwin_map(ib->tags_board);
          barwin_map_subwin(ib->tags_board);

          if(conf.border.layout)
               barwin_map_subwin(ib->layout_button);

          if(conf.bars.selbar)
               barwin_map(ib->selbar);

          barwin_refresh_color(ib->bar);
          barwin_refresh(ib->bar);

          /* Default statustext is set here */
          ib->statustext = xstrdup("wmfs"WMFS_VERSION);

          infobar_draw(ib);
     }

     return;
}

/** Draw the layout button in the InfoBar
 *\param i InfoBar pointer
 */
void
infobar_draw_layout(InfoBar *i)
{
     int w, sc = i->screen;

     if(!conf.layout_placement)
          barwin_move(i->layout_button, i->tags_board->geo.width + (PAD >> 1), 0);

     w = (conf.layout_button_width >= 1)
          ? conf.layout_button_width
          : (int)(textw(tags[sc][seltag[sc]].layout.symbol) + PAD);

     barwin_resize(i->layout_button, w, i->geo.height);
     barwin_refresh_color(i->layout_button);

     /* Split mode indicator; little rectangle at bottom-right */
     if(tags[sc][seltag[sc]].flags & SplitFlag)
          draw_rectangle(i->layout_button->dr,
                    w - SPLIT_IND_S,
                    i->geo.height - SPLIT_IND_S,
                    SPLIT_IND_S, SPLIT_IND_S,
                    getcolor(i->layout_button->fg));

     if(tags[sc][seltag[sc]].layout.symbol)
          barwin_draw_text(i->layout_button, (PAD >> 1), FHINFOBAR, tags[sc][seltag[sc]].layout.symbol);

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
     infobar_draw_selbar(i);
     barwin_refresh_color(i->bar);
     statustext_handle(i->screen, i->statustext);

     return;
}

/** Draw Selbar (selected client title bar in infobar
 *\param sc Screen Number
 */
void
infobar_draw_selbar(InfoBar *i)
{
     char *str = NULL;
     int sc = i->screen;

     if(!conf.bars.selbar)
          return;

     if(!sel || (sel && sel->screen != sc))
     {
          barwin_unmap(i->selbar);
          return;
     }
     else if(sel)
          barwin_map(i->selbar);

     /* Truncate string if too long */
     if(conf.selbar.maxlength >= 0 && sel && sel->title)
     {
          str = xcalloc(conf.selbar.maxlength + 4, sizeof(char));
          strncpy(str, sel->title, conf.selbar.maxlength);

          if(strlen(sel->title) > (size_t)conf.selbar.maxlength)
               strcat(str, "...");
     }

     barwin_resize(i->selbar, textw(str ? str : sel->title) + PAD, i->geo.height - 2);
     barwin_move(i->selbar,
               ((conf.layout_placement)
                ? (i->tags_board->geo.x + i->tags_board->geo.width + (PAD >> 1))
                : (i->layout_button->geo.x + i->layout_button->geo.width + (PAD >> 1))), 1);

     barwin_refresh_color(i->selbar);
     barwin_draw_text(i->selbar, (PAD >> 1), FHINFOBAR - 1, ((str) ? str : sel->title));

     barwin_refresh(i->selbar);

     free(str);

     return;
}

/** Draw the taglist in the InfoBar
 *\param i InfoBar pointer
*/
void
infobar_draw_taglist(InfoBar *i)
{
     int j, x, sc = i->screen;
     Client *c = SLIST_FIRST(&clients);
     uint occupied = 0;

     if(conf.layout_placement)
          barwin_move(i->tags_board,
                    ((conf.layout_button_width > 0)
                     ? (uint)conf.layout_button_width
                     : (textw(tags[sc][seltag[sc]].layout.symbol) + PAD)) + (PAD >> 1), 0);

     SLIST_FOREACH(c, &clients, next)
          if(c->screen == sc)
               occupied |= TagFlag(c->tag);

     for(j = 1, x = 0; j < conf.ntag[sc] + 1; ++j)
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
               barwin_resize(i->tags_board, (x += i->tags[j]->geo.width), i->geo.height);
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
          if(conf.bars.selbar)
               barwin_delete(infobar[sc].selbar);

          barwin_delete_subwin(infobar[sc].bar);
          barwin_delete(infobar[sc].bar);
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
     infobar_draw(&infobar[selscreen]);
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
