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
     int i, j = 0;

     if(!infobar)
          infobar = emalloc(1, sizeof(InfoBar));
     infobar->geo.height = font->height * 1.5;
     infobar->geo.y = (conf.bartop) ? 0 : MAXH - infobar->geo.height;

     /* Create infobar barwindow */
     infobar->bar = barwin_create(root, 0, infobar->geo.y, MAXW, infobar->geo.height, conf.colors.bar, False);


     /* Create tags window */
     for(i = 1; i < conf.ntag + 1; ++i)
     {
          infobar->tags[i] = barwin_create(infobar->bar->win, j, 0, textw(tags[i].name) + PAD,
                                           infobar->geo.height, conf.colors.bar, False);
          j += textw(tags[i].name) + PAD;
          barwin_map_subwin(infobar->tags[i]);
     }

     /* Create layout switch & layout type switch barwindow */
     infobar->layout_button = barwin_create(infobar->bar->win, j + PAD / 2, 0,
                                         textw(tags[seltag].layout.symbol) + PAD,
                                         infobar->geo.height, conf.colors.layout_bg, False);

     /* Map/Refresh all */
     barwin_map(infobar->bar);
     barwin_map_subwin(infobar->bar);
     barwin_map_subwin(infobar->layout_button);
     barwin_refresh_color(infobar->bar);
     barwin_refresh(infobar->bar);

     strcpy(infobar->statustext, "WMFS-" WMFS_VERSION);
     infobar_draw();

     return;
}

/** Draw the Infobar
*/
void
infobar_draw(void)
{
     infobar_draw_taglist();
     infobar_draw_layout();
     barwin_refresh_color(infobar->bar);

     /* Draw status text */
     draw_text(infobar->bar->dr,
               (MAXW - SHADH) - textw(infobar->statustext),
               font->height,
               conf.colors.text, 0,
               infobar->statustext);

     barwin_refresh(infobar->bar);

     return;
}

/** Draw the layout button in the InfoBar
 */
void
infobar_draw_layout(void)
{
     barwin_resize(infobar->layout_button, textw(tags[seltag].layout.symbol) + PAD, infobar->geo.height);
     barwin_refresh_color(infobar->layout_button);
     draw_text(infobar->layout_button->dr, PAD / 2, font->height,
               conf.colors.layout_fg, 0, tags[seltag].layout.symbol);
     barwin_refresh(infobar->layout_button);

     return;
}

/** Draw the taglist in the InfoBar
*/
void
infobar_draw_taglist(void)
{
     int i;

     for(i = 1; i < conf.ntag + 1; ++i)
     {
          infobar->tags[i]->color = ((i == seltag) ? conf.colors.tagselbg : conf.colors.bar);
          barwin_refresh_color(infobar->tags[i]);
          draw_text(infobar->tags[i]->dr, PAD / 2, font->height,
                    ((i == seltag) ? conf.colors.tagselfg : conf.colors.text),
                    0, tags[i].name);
          barwin_refresh(infobar->tags[i]);
     }

     return;
}

/** Destroy the InfoBar
*/
void
infobar_destroy(void)
{
     int i;

     barwin_delete(infobar->bar);
     barwin_delete(infobar->layout_button);
     barwin_delete_subwin(infobar->layout_button);
     for(i = 1; i < conf.ntag + 1; ++i)
     {
          barwin_delete_subwin(infobar->tags[i]);
          barwin_delete(infobar->tags[i]);
     }
     barwin_delete_subwin(infobar->bar);

     return;
}


/** Toggle the infobar position
 * \param cmd uicb_t type unused
*/
void
uicb_infobar_togglepos(uicb_t cmd)
{
     conf.bartop = !conf.bartop;

     if(conf.bartop)
          sgeo.y = infobar->geo.height + TBARH;
     else
          sgeo.y = TBARH;

     infobar->geo.y = (conf.bartop) ? 0 : MAXH - infobar->geo.height;
     barwin_move(infobar->bar, 0, infobar->geo.y);
     infobar_draw();
     arrange();

     return;
}
