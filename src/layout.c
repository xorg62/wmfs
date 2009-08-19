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
arrange(int screen, Bool update_layout)
{
     Client *c;

     if(screen < 0 || screen > screen_count())
          screen = screen_get_sel();

     for(c = clients; c; c = c->next)
          if(c->screen == screen)
          {
               if(!ishide(c, screen))
                    client_unhide(c);
               else
                    client_hide(c);
          }

     if(update_layout)
          tags[screen][seltag[screen]].layout.func(screen);

     infobar_draw(screen);

     return;
}

/** The free layout function
*/
void
freelayout(int screen)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c, selscreen)
             && c->screen == screen_get_sel()
             && !c->max)
          {
               client_moveresize(c, c->ogeo, True);
               c->tile = c->lmax = False;
          }

     ewmh_update_current_tag_prop();

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

     ewmh_update_current_tag_prop();
     tags[selscreen][seltag[selscreen]].layout.func(selscreen);
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
maxlayout(int screen)
{
     Client *c;
     int i;

     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          c->tile = False;
          c->lmax = True;
          client_maximize(c);

          /* Focus the first client
          if(!i)
          client_focus(c); */
     }

     ewmh_update_current_tag_prop();

     return;
}

/** Sort all the client that can be
 *  tiled
 * \param c Client pointer
 * \return a client pointer
*/
Client*
tiled_client(int screen, Client *c)
{
     for(;c && (c->max
                || c->free
                || c->screen != screen
                || c->state_fullscreen
                || ishide(c, screen)); c = c->next);

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
     tags[selscreen][seltag[selscreen]].layout.func(selscreen);

     ewmh_update_current_tag_prop();

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

     for(nc = 0, c = tiled_client(selscreen, clients); c; c = tiled_client(selscreen, c->next), ++nc);

     if(!nc || tags[selscreen][seltag[selscreen]].nmaster + n == 0
        || tags[selscreen][seltag[selscreen]].nmaster + n > nc)
          return;

     tags[selscreen][seltag[selscreen]].nmaster += n;
     tags[selscreen][seltag[selscreen]].layout.func(selscreen);

     ewmh_update_current_tag_prop();

     return;
}

/** Grid layout function
*/
void
grid(int screen)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
     unsigned int i, n, cols, rows, cpcols = 0;

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK(n);

     for(rows = 0; rows <= n / 2; ++rows)
          if(rows * rows >= n)
               break;

     cols = (rows && ((rows - 1) * rows) >= n)
          ? rows - 1
          : rows;

     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          /* Set client property */
          c->max = c->lmax = False;
          c->tile = True;
          ++cpcols;
          cgeo.width = (sg.width / cols) - (BORDH * 2);
          cgeo.height = (sg.height / rows) - BORDH;

          /* Last row's and last client remainder */
          if(cpcols == rows || i + 1 == n)
               cgeo.height = sg.y + sg.height - cgeo.y - BORDH;

          /* Last column's client remainder */
          if(i >= rows * (cols - 1))
               cgeo.width = sg.width - (cgeo.x - (sg.x - (BORDH * 2)));

          /* Resize */
          client_moveresize(c, cgeo, tags[screen][seltag[screen]].resizehint);

          /* Set all the other size with current client info */
          cgeo.y = c->geo.y + c->geo.height + BORDH + TBARH;

          if(cpcols + 1 > rows)
          {
               cpcols = 0;
               cgeo.x = c->geo.x + c->geo.width + (BORDH * 2);
               cgeo.y = sg.y;
          }
     }

     ewmh_update_current_tag_prop();

     return;
}

/** Multi tile function
 * \param type Postion type { Top, Bottom, Left, Right }
*/
void
multi_tile(int screen, Position type)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle mastergeo = {sg.x, sg.y, 0, 0};
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
     uint i, n, tilesize, mwfact, nmaster = tags[screen][seltag[screen]].nmaster;

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK(n);

     /* FIX NMASTER */
     nmaster = (n < nmaster) ? n : nmaster;

     /* SET MWFACT */
     mwfact = (type == Top || type == Bottom)
          ? tags[screen][seltag[screen]].mwfact * sg.height
          : tags[screen][seltag[screen]].mwfact * sg.width;

     /* MASTER SIZE */
     if(type == Top || type == Bottom)
     {
          if(type == Top)
               mastergeo.y = (n <= nmaster) ? sg.y : sg.y + (sg.height - mwfact) - BORDH;
          mastergeo.width = (sg.width / nmaster) - (BORDH * 4);
          mastergeo.height = (n <= nmaster) ? sg.height - BORDH : mwfact;
     }
     else
     {
          if(type == Left)
               mastergeo.x = (n <= nmaster) ? sg.x : (sg.x + sg.width) - mwfact - (BORDH * 2);
          mastergeo.width = (n <= nmaster) ? sg.width - (BORDH * 2) : mwfact;
          mastergeo.height = (sg.height / nmaster) - BORDH;
     }

     /* TILED SIZE */
     if(n > nmaster)
     {
          if(type == Top || type == Bottom)
               tilesize = sg.width / (n - nmaster) - (BORDH * 4);
          else
               tilesize = sg.height / (n - nmaster) - ((BORDH * 2) + TBARH);
     }


     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
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
                    cgeo.height -= (TBARH + BORDH);
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
                         cgeo.y += mastergeo.height + TBARH + BORDH;
                         cgeo.x = sg.x;
                         break;
                    default:
                    case Right:
                         cgeo.x += mastergeo.width + (BORDH * 2);
                         cgeo.y = sg.y;
                         break;
                    }
               }
               if(type == Top || type == Bottom)
               {
                    cgeo.width = tilesize;
                    cgeo.height = sg.height - mastergeo.height - TBARH - (BORDH * 2);
               }
               else
               {
                    cgeo.width = sg.width - mastergeo.width - (BORDH * 4);
                    cgeo.height = tilesize;
               }
          }

          /* REMAINDER */
          if(i + 1 == n  || i + 1 == (n < nmaster ? n : nmaster))
          {
               if(type == Top || type == Bottom)
                    cgeo.width = sg.width - (cgeo.x - (sg.x - (BORDH * 2)));
               else
                    cgeo.height = (sg.y + sg.height) - cgeo.y - BORDH;
          }

          /* Magic instant */
          client_moveresize(c, cgeo, tags[screen][seltag[screen]].resizehint);

          /* Set the position of the next client */
          if(type == Top || type == Bottom)
               cgeo.x = c->geo.x + c->geo.width + (BORDH * 2);
          else
               cgeo.y = c->geo.y + c->geo.height + BORDH + TBARH;
     }

     ewmh_update_current_tag_prop();

     return;
}

/** Mirror layout function
 * \param screen Screen to execute this function
 * \param horizont To specify the mirror mode (vertical/horizontal)
 */
void
mirror(int screen, Bool horizontal)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle mastergeo = {sg.x, sg.y, sg.width, sg.height};
     XRectangle cgeo = {sg.x, sg.y , sg.width, sg.height};
     XRectangle nextg[2] = { {0} };
     uint i, n, tilesize, mwfact;
     uint nmaster = tags[screen][seltag[screen]].nmaster;
     int pa, imp;
     Bool isp = 0;

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK(n);

     /* Fix nmaster */
     nmaster = (n < nmaster) ? n : nmaster;

     imp = ((n - (nmaster - 1)) / 2);
     pa = ((n - (nmaster - 1)) / 2) - (((n - (nmaster - 1)) % 2) ? 0 : 1);

     /* Set mwfact */
     if(tags[screen][seltag[screen]].mwfact < 0.55)
          tags[screen][seltag[screen]].mwfact = 0.55;

     mwfact = tags[screen][seltag[screen]].mwfact * ((horizontal) ? sg.height : sg.width);

     /* Master size */
     if(horizontal)
     {
          mastergeo.width = (sg.width / nmaster) - (BORDH * 2);
          mastergeo.height -= BORDH;
     }
     else
     {
          mastergeo.width -= BORDH * 2;
          mastergeo.height = (sg.height / nmaster) - (TBARH + (BORDH * 2));
     }

     if(n == nmaster + 1)
     {
          if(horizontal)
          {
               mastergeo.height = mwfact - ((BORDH * 2) + TBARH);
               tilesize = (sg.height - mastergeo.height) - ((BORDH * 2) + TBARH);
          }
          else
          {
               mastergeo.width = mwfact - (BORDH * 3);
               tilesize = (sg.width - mastergeo.width) - (BORDH * 4);
          }
     }
     if(n > nmaster + 1)
     {
          if(horizontal)
          {
               mastergeo.y = (sg.y + (sg.height - mwfact)) + TBARH + BORDH;
               mastergeo.height = (2 * mwfact - sg.height) - ((BORDH * 3) + (TBARH * 2));
               tilesize = (mwfact - mastergeo.height) - ((BORDH * 3) + (TBARH * 2));
          }
          else
          {
               mastergeo.x = (sg.x + (sg.width - mwfact)) + BORDH;
               mastergeo.width = ((2 * mwfact - sg.width) - (BORDH * 4));
               tilesize = (mwfact - mastergeo.width) - (BORDH * 5);
          }
     }

     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          /* Set client property */
          c->max = c->lmax = False;
          c->tile = True;

          if(i < nmaster)
          {
               cgeo = mastergeo;

               /* Master remainder */
               if(i + 1 == nmaster)
               {
                    if(horizontal)
                         cgeo.width = (sg.x + sg.width) - (cgeo.x + (BORDH * 2));
                    else
                         cgeo.height = (sg.y + sg.height) - (cgeo.y + BORDH);
               }
          }
          else
          {
               if(horizontal)
                    cgeo.height = tilesize;
               else
                    cgeo.width = tilesize;

               if((i + nmaster) % 2)
               {
                    isp = 1;

                    if(horizontal)
                    {
                         cgeo.y = sg.y;
                         cgeo.width = (sg.width / pa) - (BORDH * 2);
                    }
                    else
                    {
                         cgeo.x = sg.x;
                         cgeo.height = (sg.height / pa) - (TBARH + (BORDH * 2));
                    }
               }
               else
               {
                    isp = 0;

                    if(horizontal)
                    {
                         cgeo.y = (sg.y + mwfact) - BORDH;
                         cgeo.width = (sg.width / imp) - (BORDH * 2);
                    }
                    else
                    {
                         cgeo.x = (sg.x + mwfact) - BORDH;
                         cgeo.height = (sg.height / imp) - (TBARH + (BORDH * 2));
                    }
               }

               /* Remainder */
               if(i + 1 == n || i + 1 == n - 1)
               {
                    if(horizontal)
                         cgeo.width = (sg.x + sg.width) - (cgeo.x + (BORDH * 2));
                    else
                         cgeo.height = (sg.y + sg.height) - (cgeo.y + BORDH);
               }
          }

          client_moveresize(c, cgeo, tags[screen][seltag[screen]].resizehint);

          if(i >= nmaster)
               nextg[!isp] = c->geo;

          /* Next y/x position */
          if(i >= nmaster - 1)
          {
               if(horizontal)
               {
                    if(i == nmaster || i == nmaster - 1)
                         cgeo.x = sg.x;
                    else
                         cgeo.x = nextg[isp].x + nextg[isp].width + BORDH * 2;
               }
               else
               {
                    if(i == nmaster || i == nmaster - 1)
                         cgeo.y = sg.y;
                    else
                         cgeo.y = nextg[isp].y + nextg[isp].height + BORDH + TBARH;
               }
          }
          else if (i <= nmaster - 1)
          {
               if(horizontal)
                    mastergeo.x = c->geo.x + c->geo.width + BORDH * 2;
               else
                    mastergeo.y = c->geo.y + c->geo.height + BORDH + TBARH;
          }

     }

     ewmh_update_current_tag_prop();

     return;
}

/** Tile Right function
*/
void
tile(int screen)
{
     multi_tile(screen, Right);

     return;
}

/** Tile Left function
*/
void
tile_left(int screen)
{
     multi_tile(screen, Left);

     return;
}

/** Tile Top function
*/
void
tile_top(int screen)
{
     multi_tile(screen, Top);

     return;
}

/** Tile Bottom function
*/
void
tile_bottom(int screen)
{
     multi_tile(screen, Bottom);

     return;
}

/** Mirror tile vertical function
 */
void
mirror_vertical(int screen)
{
     mirror(screen, False);

     return;
}

/** Mirror tile horizontal function
 */
void
mirror_horizontal(int screen)
{
     mirror(screen, True);

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
     if((c = sel) == tiled_client(selscreen, clients))
          CHECK((c = tiled_client(selscreen, c->next)));
     client_detach(c);
     client_attach(c);
     client_focus(c);
     tags[selscreen][seltag[selscreen]].layout.func(selscreen);

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
          client_raise(sel);
     }
     else
          sel->ogeo = sel->geo;

     client_update_attributes(sel);

     tags[selscreen][seltag[selscreen]].layout.func(selscreen);

     return;
}


/** Toggle the selected client to max
 * \param cmd uicb_t type unused
*/
void
uicb_togglemax(uicb_t cmd)
{
     if(!sel || ishide(sel, selscreen) || sel->hint || sel->state_fullscreen)
          return;

     if(!sel->max)
     {
          sel->ogeo = sel->geo;
          sel->tile = False;
          sel->free = False;
          client_maximize(sel);
          client_raise(sel);
          sel->max = True;
     }
     else
     {
          sel->max = False;
          tags[selscreen][seltag[selscreen]].layout.func(selscreen);
     }

     return;
}

/** Toggle the resizehint
 * \param cmd uicb_t type
 */
void
uicb_toggle_resizehint(uicb_t cmd)
{
     screen_get_sel();

     tags[selscreen][seltag[selscreen]].resizehint = !tags[selscreen][seltag[selscreen]].resizehint;

     tags[selscreen][seltag[selscreen]].layout.func(selscreen);

     return;
}

/** Set the layout
 * \param cmd uicb_t type
*/
void
uicb_set_layout(uicb_t cmd)
{
     int i, j, n;

     screen_get_sel();

     /* Set layout_list lenght */
     for(n = 0; layout_list[n].name != NULL && layout_list[n].func != NULL; ++n);

     for(i = 0; i < n; ++i)
          if(!strcmp(cmd, _strdup(layout_list[i].name)))
               for(j = 0; j < LEN(conf.layout); ++j)
                    if(layout_list[i].func == conf.layout[j].func)
                         tags[selscreen][seltag[selscreen]].layout = conf.layout[j];

     arrange(selscreen, True);

     return;
}
