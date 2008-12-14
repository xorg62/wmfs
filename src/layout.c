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

     screen_get_sel();

     for(c = clients; c; c = c->next)
          if(c->screen == selscreen)
          {
               if(!ishide(c))
                    client_unhide(c);
               else
                    client_hide(c);
          }

     tags[selscreen][seltag[selscreen]].layout.func();
     infobar_draw(selscreen);

     return;
}

/** The free layout function
*/
void
freelayout(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c) && c->screen == screen_get_sel())
          {
               client_moveresize(c, c->ogeo, True);
               c->tile = c->lmax = False;
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
     Client *c;

     screen_get_sel();

     if(tags[selscreen][seltag[selscreen]].layout.func == freelayout)
          for(c = clients; c && (c->tag != seltag[selscreen] && c->screen != selscreen); c = c->next)
               c->ogeo = c->geo;

     for(i = 0; i < conf.nlayout; ++i)
     {
          if(tags[selscreen][seltag[selscreen]].layout.func == conf.layout[i].func
             && tags[selscreen][seltag[selscreen]].layout.symbol == conf.layout[i].symbol)
          {
               if(b)
                    tags[selscreen][seltag[selscreen]].layout = conf.layout[(i + 1) % conf.nlayout];
               else
                    tags[selscreen][seltag[selscreen]].layout = conf.layout[(i + conf.nlayout - 1) % conf.nlayout];
               break;
          }
     }
     tags[selscreen][seltag[selscreen]].layout.func();
     infobar_draw(selscreen);

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

     for(c = nexttiled(clients); c; c = nexttiled(c->next))
     {
          c->tile = False;
          c->lmax = True;
          client_maximize(c);
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
     for(;c && (c->max
                || c->free
                || c->screen != selscreen
                || c->state_fullscreen
                || ishide(c)); c = c->next);

     return c;
}

/** Set the mwfact
 * \param cmd Mwfact (string)
*/
void
uicb_set_mwfact(uicb_t cmd)
{
     double c;

     screen_get_sel();

     CHECK((sscanf(cmd, "%lf", &c)));

     if(tags[selscreen][seltag[selscreen]].mwfact + c > 0.95
        || tags[selscreen][seltag[selscreen]].mwfact + c < 0.05)
          return;

     tags[selscreen][seltag[selscreen]].mwfact += c;
     tags[selscreen][seltag[selscreen]].layout.func();

     return;
}

/** Set the nmaster
 * \param cmd Nmaster (string)
*/
void
uicb_set_nmaster(uicb_t cmd)
{
     int nc, n = atoi(cmd);
     Client *c;

     screen_get_sel();

     for(nc = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++nc);

     if(!nc || tags[selscreen][seltag[selscreen]].nmaster + n == 0
        || tags[selscreen][seltag[selscreen]].nmaster + n > nc)
          return;

     tags[selscreen][seltag[selscreen]].nmaster += n;
     tags[selscreen][seltag[selscreen]].layout.func();

     return;
}

/** Grid layout function
*/
void
grid(void)
{
     Client *c;
     XRectangle sg = sgeo[selscreen];
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
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
          ++cpcols;
          cgeo.width = (sg.width / cols) - border;
          cgeo.height = (sg.height / rows) - border;

          /* Last row's and last client remainder */
          if(cpcols == rows || i + 1 == n)
               cgeo.height =  sg.y + sg.height - cgeo.y - border;

          /* Last column's client remainder */
          if(i >= rows * (cols - 1))
               cgeo.width = sg.width - (cgeo.x - (sg.x - border));

          /* Resize */
          client_moveresize(c, cgeo, tags[selscreen][seltag[selscreen]].resizehint);

          /* Set all the other size with current client info */
          cgeo.y = c->geo.y + c->geo.height + border + TBARH;
          if(cpcols + 1 > rows)
          {
               cpcols = 0;
               cgeo.x = c->geo.x + c->geo.width + border;
               cgeo.y = sg.y;
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
     XRectangle sg = sgeo[selscreen];
     XRectangle mastergeo = {sg.x, sg.y, 0, 0};
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
     uint i , n, tilesize, mwfact, nmaster = tags[selscreen][seltag[selscreen]].nmaster;
     uint border = BORDH * 2;

     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     CHECK(n);

     /* FIX NMASTER */
     nmaster = (n < nmaster) ? n : nmaster;

     /* SET MWFACT */
     mwfact = (type == Top || type == Bottom)
          ? tags[selscreen][seltag[selscreen]].mwfact * sg.height
          : tags[selscreen][seltag[selscreen]].mwfact * sg.width;

     /* MASTER SIZE */
     if(type == Top || type == Bottom)
     {
          if(type == Top)
               mastergeo.y = (n <= nmaster) ? sg.y : sg.y + (sg.height - mwfact) - border;
          mastergeo.width = (sg.width / nmaster) - (border * 2);
          mastergeo.height = (n <= nmaster) ? sg.height - border : mwfact;
     }
     else
     {
          if(type == Left)
               mastergeo.x = (n <= nmaster) ? sg.x : (sg.x + sg.width) - mwfact - border;
          mastergeo.width = (n <= nmaster) ? sg.width - border : mwfact;
          mastergeo.height = (sg.height / nmaster) - border;
     }

     /* TILED SIZE */
     if(n > nmaster)
     {
          if(type == Top || type == Bottom)
               tilesize = sg.width / (n - nmaster) - border;
          else
               tilesize = sg.height / (n - nmaster) - border;
     }


     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++i)
     {
          /* Set client property */
          c->max = c->lmax = False;
          c->tile = True;

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
                         cgeo.y = sg.y;
                         cgeo.x = sg.x;
                         break;
                    case Bottom:
                         cgeo.y += mastergeo.height + TBARH + border;
                         cgeo.x = sg.x;
                         break;
                    default:
                    case Right:
                         cgeo.x += mastergeo.width + border;
                         cgeo.y = sg.y;
                         break;
                    }
               }
               if(type == Top || type == Bottom)
               {
                    cgeo.width = tilesize;
                    cgeo.width -= border;
                    cgeo.height = sg.height - mastergeo.height - TBARH - border*2;
               }
               else
               {
                    cgeo.width = sg.width - mastergeo.width - border*2;
                    cgeo.height = tilesize;
                    cgeo.height -= border + TBARH;
               }
          }

          /* REMAINDER */
          if(i + 1 == n  || i + 1 == (n < nmaster ? n : nmaster))
          {
               if(type == Top || type == Bottom)
                    cgeo.width = sg.width - (cgeo.x - (sg.x - border));
               else
                    cgeo.height = (sg.y + sg.height) - cgeo.y - border;
          }

          /* Magic instant */
          client_moveresize(c, cgeo, tags[selscreen][seltag[selscreen]].resizehint);

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

     screen_get_sel();

     if(!sel || sel->hint || !sel->tile || sel->state_fullscreen)
          return;
     if((c = sel) == nexttiled(clients))
          CHECK((c = nexttiled(c->next)));
     client_detach(c);
     client_attach(c);
     client_focus(c);
     tags[selscreen][seltag[selscreen]].layout.func();

     return;
}

/** Toggle the selected client to free
 * \param cmd uicb_t type unused
*/
void
uicb_togglefree(uicb_t cmd)
{
     if(!sel || sel->screen != screen_get_sel() || sel->state_fullscreen)
          return;

     sel->free = !sel->free;

     if(sel->free)
     {
          sel->tile = sel->max = sel->lmax = False;
          client_moveresize(sel, sel->ogeo, True);
     }
     else
          sel->ogeo = sel->geo;

     tags[selscreen][seltag[selscreen]].layout.func();

     return;
}


/** Toggle the selected client to max
 * \param cmd uicb_t type unused
*/
void
uicb_togglemax(uicb_t cmd)
{
     if(!sel || ishide(sel) || sel->hint || sel->state_fullscreen)
          return;

     if(!sel->max)
     {
          sel->tile = sel->free = False;
          client_maximize(sel);
          sel->max = True;
     }
     else if(sel->max)
     {
          sel->max = False;
          tags[selscreen][seltag[selscreen]].layout.func();
     }

     return;
}
