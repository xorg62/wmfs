/*
*      layout.c
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

/** Arrange All
*/
void
arrange(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c))
               client_unhide(c);
          else
               client_hide(c);

     tags[seltag].layout.func();

     if(selbytag[seltag] != NULL)
          client_focus(selbytag[seltag]);
      else
          client_focus(NULL);

     infobar_draw();

     return;
}

/** The free layout function
*/
void
freelayout(void)
{
     Client *c;
     XRectangle geo;

     for(c = clients; c; c = c->next)
     {
          if(!ishide(c))
          {
               if(c->tile || c->lmax)
               {
                    geo.x = c->ogeo.x;
                    geo.y = c->ogeo.y;
                    geo.width = c->ogeo.width;
                    geo.height = c->ogeo.height;
                    client_moveresize(c, geo, True);
                    c->tile = c->lmax = False;
               }
          }
     }

     return;
}

/** Layout switching function
 * \param b Bool True : next False : previous
*/
void
layoutswitch(Bool b)
{
     int i;

     for(i = 0; i < conf.nlayout; ++i)
     {
          if(tags[seltag].layout.symbol == conf.layout[i].symbol
             && tags[seltag].layout.func == conf.layout[i].func)
          {
               if(b)
                    tags[seltag].layout = conf.layout[(i + 1) % conf.nlayout];
               else
                    tags[seltag].layout = conf.layout[(i + conf.nlayout - 1) % conf.nlayout];
               break;
          }
     }
     arrange();

     return;
}

/** Tile layout switching function
 * \param b Bool True : next False : previous
*/
void
layout_tile_switch(Bool b)
{
     int i;

     for(i = 0; i < conf.ntilelayout; ++i)
     {
          if(tags[seltag].layout.func == conf.layout[i].func
             && (conf.layout[i].func != freelayout
                 && conf.layout[i].func != maxlayout))
          {
               if(b)
                    tags[seltag].layout = conf.layout[(i + 1) % conf.ntilelayout];
               else
                    tags[seltag].layout = conf.layout[(i + conf.ntilelayout - 1) % conf.ntilelayout];
               break;
          }
     }
     arrange();

     return;
}

/** Set the next layout
 * \param cmd uicb_t type unused
*/
void
uicb_layout_next(uicb_t cmd)
{
     layoutswitch(True);

     return;
}

/** Set the previous layout
 * \param cmd uicb_t type unused
*/
void
uicb_layout_prev(uicb_t cmd)
{
     layoutswitch(False);

     return;
}

/** Max layout function
*/
void
maxlayout(void)
{
     Client *c;
     XRectangle geo;

     for(c = nexttiled(clients); c; c = nexttiled(c->next))
     {
          c->tile = False;
          c->lmax = True;
          c->ogeo.x = c->geo.x;
          c->ogeo.y = c->geo.y;
          c->ogeo.width = c->geo.width;
          c->ogeo.height = c->geo.height;

          geo.x = sgeo.x; geo.y = sgeo.y;
          geo.width = sgeo.width - BORDH * 2;
          geo.height = sgeo.height - BORDH * 2;

          client_moveresize(c, geo, False);
     }

     return;
}

/** Sort all the client that can be
 *  tiled
 * \param c Client pointer
 * \return a client pointer
*/
Client*
nexttiled(Client *c)
{
     for(; c && (c->max || c->free || ishide(c)); c = c->next);

     return c;
}

/** Set the mwfact
 * \param cmd Mwfact
*/
void
uicb_set_mwfact(uicb_t cmd)
{
     double c;

     CHECK((sscanf(cmd, "%lf", &c)));

     if(tags[seltag].mwfact + c > 0.95
        || tags[seltag].mwfact + c < 0.05)
          return;

     tags[seltag].mwfact += c;
     arrange();

     return;
}

/** Set the nmaster
 * \param cmd nmaster
*/
void
uicb_set_nmaster(uicb_t cmd)
{
     int nc, n = atoi(cmd);
     Client *c;

     for(nc = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++nc);

     if(!nc || tags[seltag].nmaster + n == 0
        || tags[seltag].nmaster + n > nc)
          return;

     tags[seltag].nmaster += n;
     arrange();

     return;
}

/** Grid layout function
*/
void
grid(void)
{
     Client *c;
     XRectangle cgeo = {sgeo.x, sgeo.y, 0, 0};
     unsigned int i, n, cols, rows, cpcols = 0;
     unsigned int border = BORDH * 2;

     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     CHECK(n);

     for(rows = 0; rows <= n / 2; ++rows)
          if(rows * rows >= n)
               break;

     cols = (rows && ((rows - 1) * rows) >= n)
          ? rows - 1
          : rows;

     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++i)
     {
          /* Set client property */
          c->max = c->lmax = False;
          c->tile = True;
          c->ogeo.x = c->geo.x; c->ogeo.y = c->geo.y;
          c->ogeo.width = c->geo.width; c->ogeo.height = c->geo.height;

          ++cpcols;
          cgeo.width = (sgeo.width / cols) - border;
          cgeo.height = (sgeo.height / rows) - border;

          /* Last row's and last client remainder */
          if(cpcols == rows || i + 1 == n)
               cgeo.height = (sgeo.y + sgeo.height) - cgeo.y - border;

          /* Last column's client remainder */
          if(i >= rows * (cols - 1))
               cgeo.width = sgeo.width - cgeo.x - BORDH;

          /* Resize */
          client_moveresize(c, cgeo, tags[seltag].resizehint);

          /* Set all the other size with current client info */
          cgeo.y = c->geo.y + c->geo.height + border + TBARH;
          if(cpcols + 1 > rows)
          {
               cpcols = 0;
               cgeo.x = c->geo.x + c->geo.width + border;
               cgeo.y = sgeo.y;
          }
     }

     return;
}

/** Multi tile function
 * \param type Postion type { Top, Bottom, Left, Right }
*/
void
multi_tile(Position type)
{
     Client *c;
     XRectangle mastergeo = {sgeo.x, sgeo.y, 0, 0};
     XRectangle cgeo = {sgeo.x, sgeo.y, 0, 0};
     uint i , n, tilesize, mwfact, nmaster = tags[seltag].nmaster;
     uint border = BORDH * 2;

     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     CHECK(n);

     /* FIX NMASTER */
     nmaster = (n < nmaster) ? n : nmaster;

     /* SET MWFACT */
     mwfact = (type == Top || type == Bottom)
          ? tags[seltag].mwfact * sgeo.height
          : tags[seltag].mwfact * sgeo.width;

     /* MASTER SIZE */
     if(type == Top || type == Bottom)
     {
          if(type == Top)
               mastergeo.y = (n <= nmaster) ? sgeo.y : sgeo.y + (sgeo.height - mwfact) - border;
          mastergeo.width = (sgeo.width / nmaster) - BORDH;
          mastergeo.height = (n <= nmaster) ? sgeo.height - BORDH : mwfact;
     }
     else
     {
          if(type == Left)
               mastergeo.x = (n <= nmaster) ? sgeo.x : sgeo.width - mwfact - BORDH;
          mastergeo.width = (n <= nmaster) ? sgeo.width - border : mwfact;
          mastergeo.height = (sgeo.height / nmaster) - border;
     }

     /* TILED SIZE */
     if(n > nmaster)
     {
          if(type == Top || type == Bottom)
               tilesize = sgeo.width / (n - nmaster) - border;
          else
               tilesize = sgeo.height / (n - nmaster) - border;
     }


     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++i)
     {
          /* Set client property, 'don't care */
          c->max = c->lmax = False;
          c->tile = True;
          c->ogeo.x = c->geo.x; c->ogeo.y = c->geo.y;
          c->ogeo.width = c->geo.width; c->ogeo.height = c->geo.height;

          /* MASTER */
          if(i < nmaster)
          {
               cgeo.width = mastergeo.width;
               cgeo.height = mastergeo.height;

               if(type == Top || type == Bottom)
                    cgeo.y = mastergeo.y;
               else
               {
                    cgeo.x = mastergeo.x;
                    cgeo.height -= (TBARH + border);
               }
          }

          /* TILED */
          else
          {
               if(i == nmaster)
               {
                    switch(type)
                    {
                    case Top:
                    case Left:
                         cgeo.y = sgeo.y;
                         cgeo.x = sgeo.x;
                         break;
                    case Bottom:
                         cgeo.y += mastergeo.height + TBARH + border;
                         cgeo.x = sgeo.x;
                         break;
                    default:
                    case Right:
                         cgeo.x += mastergeo.width + border;
                         cgeo.y = sgeo.y;
                         break;
                    }
               }
               if(type == Top || type == Bottom)
               {
                    cgeo.width = tilesize;
                    cgeo.width -= border;
                    cgeo.height = sgeo.height - mastergeo.height - TBARH - border*2;
               }
               else
               {
                    cgeo.width = sgeo.width - mastergeo.width - border*2;
                    cgeo.height = tilesize;
                    cgeo.height -= border + TBARH;
               }
          }

          /* REMAINDER */
          if(i + 1 == n  || i + 1 == (n < nmaster ? n : nmaster))
          {
               if(type == Top || type == Bottom)
                    cgeo.width = sgeo.width - cgeo.x - BORDH;
               else
                    cgeo.height = (sgeo.y + sgeo.height) - cgeo.y - border;
          }

          /* Magic instant */
          client_moveresize(c, cgeo, tags[seltag].resizehint);

          /* Set the position of the next client */
          if(type == Top || type == Bottom)
               cgeo.x = c->geo.x + c->geo.width + border;
          else
               cgeo.y = c->geo.y + c->geo.height + border + TBARH;
     }

     return;
}

/** Tile Right function
*/
void
tile(void)
{
     multi_tile(Right);

     return;
}

/** Tile Left function
*/
void
tile_left(void)
{
     multi_tile(Left);

     return;
}

/** Tile Top function
*/
void
tile_top(void)
{
     multi_tile(Top);

     return;
}


/** Tile Bottom function
*/
void
tile_bottom(void)
{
     multi_tile(Bottom);

     return;
}

/** Put the selected client to the master postion
 * \param cmd uicb_t type unused
*/
void
uicb_tile_switch(uicb_t cmd)
{
     Client *c;

     if(!sel || sel->hint || !sel->tile)
          return;
     if((c = sel) == nexttiled(clients))
          CHECK((c = nexttiled(c->next)));
     client_detach(c);
     client_attach(c);
     client_focus(c);
     arrange();

     return;
}

/** Toggle the selected client to free
 * \param cmd uicb_t type unused
*/
void
uicb_togglefree(uicb_t cmd)
{
     CHECK(sel);

     sel->free = !sel->free;
     sel->tile = False;
     sel->max  = False;
     sel->lmax = False;
     if(sel->free)
     {
          sel->geo.x = sel->ogeo.x;
          sel->geo.y = sel->ogeo.y;
          sel->geo.width = sel->ogeo.width;
          sel->geo.height = sel->ogeo.height;
          client_moveresize(sel, sel->geo, True);
     }

     client_raise(sel);

     arrange();

     return;
}

/** Toggle the selected client to max
 * \param cmd uicb_t type unused
*/
void
uicb_togglemax(uicb_t cmd)
{
     XRectangle geo;

     if(!sel || ishide(sel) || sel->hint)
          return;
     if(!sel->max)
     {
          sel->ogeo.x = sel->geo.x;
          sel->ogeo.y = sel->geo.y;
          sel->ogeo.width = sel->geo.width;
          sel->ogeo.height = sel->geo.height;

          geo.x = sgeo.x; geo.y = sgeo.y;
          geo.width = sgeo.width - BORDH * 2;
          geo.height = sgeo.height - BORDH * 2;

          client_moveresize(sel, geo, False);
          client_raise(sel);
          sel->max = True;
     }
     else if(sel->max)
     {
          geo.x = sel->ogeo.x;
          geo.y = sel->ogeo.y;
          geo.width = sel->ogeo.width;
          geo.height = sel->ogeo.height;

          client_moveresize(sel, geo, False);
          sel->max = False;
     }
     arrange();

     return;
}
