/*
*      infobar.c
*      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
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
     int sc, i, j;

     if(!infobar)
          infobar = emalloc(screen_count(), sizeof(InfoBar));

     for(sc = 0; sc < screen_count(); ++sc)
     {
          j = 0;
          infobar[sc].geo.height = INFOBARH;
          infobar[sc].geo.y = (conf.bartop)
               ? sgeo[sc].y - INFOBARH - TBARH
               : sgeo[sc].height - INFOBARH;

          /* Create infobar barwindow */
          infobar[sc].bar = barwin_create(root, sgeo[sc].x - BORDH, infobar[sc].geo.y,
                                          sgeo[sc].width, infobar[sc].geo.height, conf.colors.bar, False);

          /* Create tags window */
          for(i = 1; i < conf.ntag[sc] + 1; ++i)
          {
               infobar[sc].tags[i] = barwin_create(infobar[sc].bar->win, j, 0, textw(tags[sc][i].name) + PAD,
                                                infobar[sc].geo.height, conf.colors.bar, False);
               j += textw(tags[sc][i].name) + PAD;
               barwin_map_subwin(infobar[sc].tags[i]);
          }

          /* Create layout switch & layout type switch barwindow */
          infobar[sc].layout_button = barwin_create(infobar[sc].bar->win, j + PAD / 2, 0,
                                         textw(tags[sc][seltag[sc]].layout.symbol) + PAD,
                                         infobar[sc].geo.height, conf.colors.layout_bg, False);

          /* Map/Refresh all */
          barwin_map(infobar[sc].bar);
          barwin_map_subwin(infobar[sc].bar);
          barwin_map_subwin(infobar[sc].layout_button);
          barwin_refresh_color(infobar[sc].bar);
          barwin_refresh(infobar[sc].bar);

          strcpy(statustext, "WMFS-" WMFS_VERSION);
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

     /* DRAW status text */
     draw_text(infobar[sc].bar->dr,
               (sgeo[sc].width - SHADH) - textw(statustext),
               font->height,
               conf.colors.text, 0,
               statustext);

     barwin_refresh(infobar[sc].bar);

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
     draw_text(infobar[sc].layout_button->dr, PAD / 2, font->height,
               conf.colors.layout_fg, 0, tags[sc][seltag[sc]].layout.symbol);
     barwin_refresh(infobar[sc].layout_button);

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

     for(i = 1; i < conf.ntag[sc] + 1; ++i)
     {
          infobar[sc].tags[i]->color = ((i == seltag[sc]) ? conf.colors.tagselbg : conf.colors.bar);
          barwin_refresh_color(infobar[sc].tags[i]);

          /* Colorize a tag if there are clients in this */
          for(c = clients; c; c = c->next)
          {
               if(c->screen == sc)
               {
                   infobar[sc].tags[c->tag]->color = ((c->tag == seltag[sc])
                                                      ? conf.colors.tagselbg
                                                      : conf.colors.tag_occupied_bg);
                   barwin_refresh_color(infobar[sc].tags[i]);
               }
          }

          if(tags[sc][i].name)
               draw_text(infobar[sc].tags[i]->dr,
                         PAD / 2, font->height,
                         ((i == seltag[sc]) ? conf.colors.tagselfg : conf.colors.text),
                         0, tags[sc][i].name);
          barwin_refresh(infobar[sc].tags[i]);
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
          barwin_delete_subwin(infobar[sc].bar);
          barwin_delete(infobar[sc].bar);
     }

     return;
}


/** Toggle the infobar position
 * \param cmd uicb_t type unused
*/
void
uicb_infobar_togglepos(uicb_t cmd)
{
     screen_get_sel();

     conf.bartop = !conf.bartop;

     if(conf.bartop)
     {
          sgeo[selscreen].y = INFOBARH + TBARH;
          infobar[selscreen].geo.y = sgeo[selscreen].y - (INFOBARH + TBARH);
     }
     else
     {
          sgeo[selscreen].y = TBARH;
          infobar[selscreen].geo.y = sgeo[selscreen].height + TBARH;
     }

     barwin_move(infobar[selscreen].bar, sgeo[selscreen].x - BORDH, infobar[selscreen].geo.y);
     infobar_draw(selscreen);

     arrange();

     return;
}
