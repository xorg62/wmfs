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

/** Init the Infobar
*/
void
infobar_init(void)
{
     int s, sc, i, j = 0;

     s = screen_count();

     if(!infobar)
          infobar = emalloc(s, sizeof(InfoBar));

     for(sc = 0; sc < s; ++sc)
     {
          j = 0;
          infobar[sc].geo.height = INFOBARH;

          switch(tags[sc][seltag[sc]].barpos)
          {
          case IB_Hide:
               sgeo[sc].y = spgeo[sc].y + TBARH;
               sgeo[sc].height += INFOBARH;
               infobar[sc].geo.y = -(infobar[sc].geo.height) * 2;
               break;
          case IB_Bottom:
               sgeo[sc].y = TBARH;
               infobar[sc].geo.y = spgeo[sc].y + sgeo[sc].height + TBARH;
               break;
          default:
          case IB_Top:
               sgeo[sc].y = spgeo[sc].y + INFOBARH + TBARH;
               infobar[sc].geo.y = spgeo[sc].y;
               break;
          }

          /* Create infobar barwindow */
          infobar[sc].bar = barwin_create(ROOT, sgeo[sc].x - BORDH, infobar[sc].geo.y,
                                          sgeo[sc].width, infobar[sc].geo.height,
                                          conf.colors.bar, conf.colors.text, False, False, conf.border.bar);

          infobar[sc].tags_board = barwin_create(infobar[sc].bar->win,
                    ((conf.layout_placement) ? textw(tags[sc][seltag[sc]].layout.symbol) + PAD * 1.5: 0), 0,
                    textw(tags[sc][0].name) + PAD, /* Base size, will change */
                    infobar[sc].geo.height,
                    conf.colors.bar, conf.colors.text, False, False, False);

          /* Create tags window */
          for(i = 1; i < conf.ntag[sc] + 1; ++i)
          {
               infobar[sc].tags[i] = barwin_create(infobar[sc].tags_board->win, j, 0,
                                                   textw(tags[sc][i].name) + PAD,
                                                   infobar[sc].geo.height,
                                                   conf.colors.bar, conf.colors.text, False, False, conf.border.tag);

               j += textw(tags[sc][i].name) + PAD;

               barwin_resize(infobar[sc].tags_board, j, infobar[sc].geo.height);
               barwin_map_subwin(infobar[sc].tags[i]);
          }

          /* Create layout switch barwindow */
          infobar[sc].layout_button = barwin_create(infobar[sc].bar->win,
                    ((conf.layout_placement) ? 0 : (j + PAD / 2)), 0,
                    textw(tags[sc][seltag[sc]].layout.symbol) + PAD,
                    infobar[sc].geo.height,
                    conf.colors.layout_bg, conf.colors.layout_fg,
                    False, False, conf.border.layout);

          /* Map/Refresh all */
          barwin_map(infobar[sc].bar);
          barwin_map_subwin(infobar[sc].bar);

          barwin_map(infobar[sc].tags_board);
          barwin_map_subwin(infobar[sc].tags_board);

          if(conf.border.layout)
               barwin_map_subwin(infobar[sc].layout_button);

          barwin_refresh_color(infobar[sc].bar);
          barwin_refresh(infobar[sc].bar);

          /* Default statustext is set here */
          for(i = 0; i < s; ++i)
               infobar[i].statustext = _strdup(WMFS_VERSION);
          infobar_draw(sc);
     }

     return;
}

/** Draw the Infobar
 *\param sc Screen number
*/
void
infobar_draw(int sc)
{
     infobar_draw_taglist(sc);
     infobar_draw_layout(sc);
     barwin_refresh_color(infobar[sc].bar);
     statustext_handle(sc, infobar[sc].statustext);

     return;
}

/** Draw the layout button in the InfoBar
 *\param sc Screen number
 */
void
infobar_draw_layout(int sc)
{
     barwin_resize(infobar[sc].layout_button, textw(tags[sc][seltag[sc]].layout.symbol) + PAD, infobar[sc].geo.height);
     barwin_refresh_color(infobar[sc].layout_button);

     if(tags[sc][seltag[sc]].layout.symbol)
          barwin_draw_text(infobar[sc].layout_button, PAD / 2, FHINFOBAR, tags[sc][seltag[sc]].layout.symbol);

     return;
}

/** Draw the taglist in the InfoBar
 *\param sc Screen number
*/
void
infobar_draw_taglist(int sc)
{
     int i;
     Client *c;

     if(conf.layout_placement)
          barwin_move(infobar[sc].tags_board, textw(tags[sc][seltag[sc]].layout.symbol) + PAD * 1.5, 0);

     for(i = 1; i < conf.ntag[sc] + 1; ++i)
     {
          infobar[sc].tags[i]->bg = ((i == seltag[sc]) ? conf.colors.tagselbg : conf.colors.bar);
          infobar[sc].tags[i]->fg = ((i == seltag[sc]) ? conf.colors.tagselfg : conf.colors.text);
          barwin_refresh_color(infobar[sc].tags[i]);

          /* Colorize a tag if there are clients in this */
          for(c = clients; c; c = c->next)
          {
               if(c->screen == sc)
               {
                    infobar[sc].tags[c->tag]->bg = ((c->tag == seltag[sc]) ? conf.colors.tagselbg : conf.colors.tag_occupied_bg);
                    barwin_refresh_color(infobar[sc].tags[i]);
               }
          }
          if(tags[sc][i].name)
               barwin_draw_text(infobar[sc].tags[i], PAD / 2, FHINFOBAR, tags[sc][i].name);
     }

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
          infobar[selscreen].geo.y = -(infobar[selscreen].geo.height) * 2;
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
     infobar_draw(selscreen);
     ewmh_set_workarea();
     arrange(selscreen, True);

     return;
}

/** Toggle the infobar position
 * \param cmd uicb_t type unused
*/
void
uicb_infobar_togglepos(uicb_t cmd)
{
     screen_get_sel();

     infobar_set_position((tags[selscreen][seltag[selscreen]].barpos
                           = (tags[selscreen][seltag[selscreen]].barpos < 2)
                           ? tags[selscreen][seltag[selscreen]].barpos + 1
                           : 0));

     return;
}
