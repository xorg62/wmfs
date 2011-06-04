/*
*      layout.c
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

/** Arrange All
*/
void
arrange(int screen, Bool update_layout)
{
     Client *c;

     if(screen < 0 || screen > screen_count() - 1)
          screen = screen_get_sel();

     for(c = clients; c; c = c->next)
          if(c->screen == screen)
          {
               if(!ishide(c, screen))
                    client_unhide(c);
               else
                    client_hide(c);
          }

     if(tags[screen][seltag[screen]].layout.func)
     {
          if(update_layout)
               layout_func(screen, seltag[screen]);

          infobar_draw(screen);
     }

     return;
}

/** Apply layout function
*/
void
layout_func(int screen, int tag)
{
     if((tags[screen][tag].flags & SplitFlag)
               && !(tags[screen][tag].flags & FirstArrangeFlag))
          split_apply_current(screen, tag);
     else
     {
          tags[screen][tag].layout.func(screen);
          tags[screen][tag].flags &= ~FirstArrangeFlag;
     }

     return;
}

/** The free layout function
*/
void
freelayout(int screen)
{
     Client *c;
     (void)screen;

     for(c = clients; c; c = c->next)
          if(!ishide(c, selscreen)
             && c->screen == screen_get_sel()
             && !(c->flags & MaxFlag))
          {
               client_moveresize(c, c->free_geo, True);
               c->flags &= ~(TileFlag | LMaxFlag);
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
          for(c = clients; c && (c->tag != (uint)seltag[selscreen] && c->screen != selscreen); c = c->next)
          {
               c->ogeo = c->geo;
               c->free_geo = c->geo;
          }

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
     tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
     tags[selscreen][seltag[selscreen]].flags &= ~SplitFlag;
     layout_func(selscreen, seltag[selscreen]);
     infobar_draw(selscreen);

     return;
}

/** Set the next layout
 * \param cmd uicb_t type unused
*/
void
uicb_layout_next(uicb_t cmd)
{
     (void)cmd;
     layoutswitch(True);

     return;
}

/** Set the previous layout
 * \param cmd uicb_t type unused
*/
void
uicb_layout_prev(uicb_t cmd)
{
     (void)cmd;
     layoutswitch(False);

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
     for(;c && ((c->flags & (MaxFlag | FreeFlag | FSSFlag | AboveFlag))
                    || c->screen != screen
                    || ishide(c, screen)); c = c->next);

     if(c)
          c->flags |= FLayFlag;

     return c;
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
          c->flags &= ~TileFlag;
          c->flags |= LMaxFlag;
          client_maximize(c);
     }

     ewmh_update_current_tag_prop();

     return;
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
     layout_func(selscreen, seltag[selscreen]);

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
     tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
     layout_func(selscreen, seltag[selscreen]);

     ewmh_update_current_tag_prop();

     return;
}

/** Grid layout function
*/
static void
grid(int screen, Bool horizontal)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
     unsigned int i, n, temp, cols, rows, cpcols = 0;

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK((tags[screen][seltag[screen]].nclients = n));

     for(rows = 0; rows <= (n >> 1); ++rows)
          if(rows * rows >= n)
               break;

     cols = (rows && ((rows - 1) * rows) >= n)
          ? rows - 1
          : rows;

     if(!horizontal)
     {
         temp = cols;
         cols = rows;
         rows = temp;
     }

     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          /* Set client property */
          c->flags &= ~(MaxFlag | LMaxFlag);
          c->flags |= TileFlag;
          ++cpcols;

          cfactor_clean(c);

          cgeo.width = (sg.width / cols) - (BORDH << 1);
          cgeo.height = (sg.height / rows) - BORDH;

          /* Last row's and last client remainder */
          if(cpcols == rows || i + 1 == n)
               cgeo.height = sg.y + sg.height - cgeo.y - BORDH;

          /* Last column's client remainder */
          if(i >= rows * (cols - 1))
               cgeo.width = sg.width - (cgeo.x - (sg.x - (BORDH << 1)));

          /* Resize */
          client_moveresize(c, (c->pgeo = cgeo), (tags[screen][seltag[screen]].flags & ResizeHintFlag));

          /* Set all the other size with current client info */
          cgeo.y = c->pgeo.y + c->pgeo.height + BORDH + TBARH;

          if(cpcols + 1 > rows)
          {
               cpcols = 0;
               cgeo.x = c->pgeo.x + c->pgeo.width + (BORDH << 1);
               cgeo.y = sg.y;
          }
     }

     tags[screen][seltag[screen]].flags &= ~CleanFactFlag;
     ewmh_update_current_tag_prop();

     return;
}

/** Multi tile function
 * \param type Postion type { Top, Bottom, Left, Right }
*/
static void
multi_tile(int screen, Position type)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle mastergeo = {sg.x, sg.y, 0, 0};
     XRectangle cgeo = {sg.x, sg.y, 0, 0};
     uint i, n, tilesize = 0, mwfact, nmaster = tags[screen][seltag[screen]].nmaster;

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK((tags[screen][seltag[screen]].nclients = n));

     /* FIX NMASTER */
     nmaster = (n < nmaster) ? n : nmaster;

     /* SET MWFACT */
     mwfact = (type == Top || type == Bottom)
          ? tags[screen][seltag[screen]].mwfact * sg.height
          : tags[screen][seltag[screen]].mwfact * sg.width;

     /* MASTER SIZE */
     if(LDIR(type))
     {
          if(type == Left)
               mastergeo.x = (n <= nmaster) ? (uint)sg.x : (sg.x + sg.width) - mwfact - (BORDH << 1);
          mastergeo.width = (n <= nmaster) ? (uint)(sg.width - (BORDH << 1)) : mwfact;
          mastergeo.height = (sg.height / nmaster) - BORDH;
     }
     else
     {
          if(type == Top)
               mastergeo.y = (n <= nmaster) ? (uint)sg.y : sg.y + (sg.height - mwfact) - BORDH;
          mastergeo.width = (sg.width / nmaster) - (BORDH << 2);
          mastergeo.height = (n <= nmaster) ? (uint)(sg.height - BORDH) : mwfact;
     }
     /* TILED SIZE */
     if(n > nmaster)
     {
          if(LDIR(type))
               tilesize = sg.height / (n - nmaster) - ((BORDH << 1) + TBARH);
          else
               tilesize = sg.width / (n - nmaster) - (BORDH << 2);
     }


     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          /* Set client property */
          c->flags &= ~(MaxFlag | LMaxFlag);
          c->flags |= TileFlag;

          cfactor_clean(c);

          /* MASTER */
          if(i < nmaster)
          {
               cgeo.width = mastergeo.width;
               cgeo.height = mastergeo.height;

               if(LDIR(type))
               {
                    cgeo.x = mastergeo.x;
                    cgeo.height -= (TBARH + BORDH);
               }
               else
                    cgeo.y = mastergeo.y;
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
                         cgeo.x += mastergeo.width + (BORDH << 1);
                         cgeo.y = sg.y;
                         break;
                    }
               }
               if(LDIR(type))
               {
                    cgeo.width = sg.width - mastergeo.width - (BORDH << 2);
                    cgeo.height = tilesize;
               }
               else
               {
                    cgeo.width = tilesize;
                    cgeo.height = sg.height - mastergeo.height - TBARH - (BORDH << 1);
               }
          }

          /* REMAINDER */
          if(i + 1 == n  || i + 1 == (n < nmaster ? n : nmaster))
          {
               if(LDIR(type))
                    cgeo.height = (sg.y + sg.height) - cgeo.y - BORDH;
               else
                    cgeo.width = sg.width - (cgeo.x - (sg.x - (BORDH << 1)));
          }

          /* Magic instant */
          client_moveresize(c, (c->pgeo = cgeo), (tags[screen][seltag[screen]].flags & ResizeHintFlag));

          /* Set the position of the next client */
          if(LDIR(type))
               cgeo.y = c->pgeo.y + c->pgeo.height + BORDH + TBARH;
          else
               cgeo.x = c->pgeo.x + c->pgeo.width + (BORDH << 1);
     }

     tags[screen][seltag[screen]].flags &= ~CleanFactFlag;
     ewmh_update_current_tag_prop();

     return;
}

/** Mirror layout function
 * \param screen Screen to execute this function
 * \param horizont To specify the mirror mode (vertical/horizontal)
 */
static void
mirror(int screen, Bool horizontal)
{
     Client *c;
     XRectangle sg = sgeo[screen];
     XRectangle mastergeo = {sg.x, sg.y, sg.width, sg.height};
     XRectangle cgeo = {sg.x, sg.y , sg.width, sg.height};
     XRectangle nextg[2];
     uint i, n, tilesize = 0, mwfact;
     uint nmaster = tags[screen][seltag[screen]].nmaster;
     int pa, imp;
     Bool isp = False;

     memset(nextg, 0, sizeof(nextg));

     for(n = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++n);
     CHECK((tags[screen][seltag[screen]].nclients = n));

     /* Fix nmaster */
     nmaster = (n < nmaster) ? n : nmaster;

     imp = ((n - (nmaster - 1)) >> 1);
     pa = ((n - (nmaster - 1)) >> 1) - (((n - (nmaster - 1)) & 1) ? 0 : 1);

     /* Set mwfact */
     if(tags[screen][seltag[screen]].mwfact < 0.55)
          tags[screen][seltag[screen]].mwfact = 0.55;

     mwfact = tags[screen][seltag[screen]].mwfact * ((horizontal) ? sg.height : sg.width);

     /* Master size */
     if(horizontal)
     {
          mastergeo.width = (sg.width / nmaster) - (BORDH << 1);
          mastergeo.height -= BORDH;
     }
     else
     {
          mastergeo.width -= (BORDH << 1);
          mastergeo.height = (sg.height / nmaster) - (TBARH + (BORDH << 1));
     }

     if(n == nmaster + 1)
     {
          if(horizontal)
          {
               mastergeo.height = mwfact - ((BORDH << 1) + TBARH);
               tilesize = (sg.height - mastergeo.height) - ((BORDH << 1) + TBARH);
          }
          else
          {
               mastergeo.width = mwfact - (BORDH * 3);
               tilesize = (sg.width - mastergeo.width) - (BORDH << 2);
          }
     }
     if(n > nmaster + 1)
     {
          if(horizontal)
          {
               mastergeo.y = (sg.y + (sg.height - mwfact)) + TBARH + BORDH;
               mastergeo.height = ((mwfact << 1) - sg.height) - ((BORDH * 3) + (TBARH << 1));
               tilesize = (mwfact - mastergeo.height) - ((BORDH * 3) + (TBARH << 1));
          }
          else
          {
               mastergeo.x = (sg.x + (sg.width - mwfact)) + BORDH;
               mastergeo.width = ((mwfact << 1) - sg.width) - (BORDH << 2);
               tilesize = (mwfact - mastergeo.width) - (BORDH * 5);
          }
     }

     for(i = 0, c = tiled_client(screen, clients); c; c = tiled_client(screen, c->next), ++i)
     {
          /* Set client property */
          c->flags &= ~(MaxFlag | LMaxFlag);
          c->flags |= TileFlag;

          cfactor_clean(c);

          if(i < nmaster)
          {
               cgeo = mastergeo;

               /* Master remainder */
               if(i + 1 == nmaster)
               {
                    if(horizontal)
                         cgeo.width = (sg.x + sg.width) - (cgeo.x + (BORDH << 1));
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

               if((i + nmaster) & 1)
               {
                    isp = 1;

                    if(horizontal)
                    {
                         cgeo.y = sg.y;
                         cgeo.width = (sg.width / pa) - (BORDH << 1);
                    }
                    else
                    {
                         cgeo.x = sg.x;
                         cgeo.height = (sg.height / pa) - (TBARH + (BORDH << 1));
                    }
               }
               else
               {
                    isp = 0;

                    if(horizontal)
                    {
                         cgeo.y = (sg.y + mwfact) - BORDH;
                         cgeo.width = (sg.width / imp) - (BORDH << 1);
                    }
                    else
                    {
                         cgeo.x = (sg.x + mwfact) - BORDH;
                         cgeo.height = (sg.height / imp) - (TBARH + (BORDH << 1));
                    }
               }

               /* Remainder */
               if(i + 1 == n || i + 1 == n - 1)
               {
                    if(horizontal)
                         cgeo.width = (sg.x + sg.width) - (cgeo.x + (BORDH << 1));
                    else
                         cgeo.height = (sg.y + sg.height) - (cgeo.y + BORDH);
               }
          }

          client_moveresize(c, (c->pgeo = cgeo), (tags[screen][seltag[screen]].flags & ResizeHintFlag));

          if(i >= nmaster)
               nextg[!isp] = c->pgeo;

          /* Next y/x position */
          if(i >= nmaster - 1)
          {
               if(horizontal)
               {
                    if(i == nmaster || i == nmaster - 1)
                         cgeo.x = sg.x;
                    else
                         cgeo.x = nextg[isp].x + nextg[isp].width + (BORDH << 1);
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
                    mastergeo.x = cgeo.x + cgeo.width + (BORDH << 1);
               else
                    mastergeo.y = cgeo.y + cgeo.height + BORDH + TBARH;
          }

     }

     tags[screen][seltag[screen]].flags &= ~CleanFactFlag;
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

/** Horizontal grid function
 */
void
grid_horizontal(int screen)
{
    grid(screen, True);

    return;
}

/** Vertical grid function
 */
void
grid_vertical(int screen)
{
    grid(screen, False);

    return;
}

/** Toggle the selected client to free
 * \param cmd uicb_t type unused
*/
void
uicb_togglefree(uicb_t cmd)
{
     (void)cmd;

     if(!sel || sel->screen != screen_get_sel() || (sel->flags & FSSFlag))
          return;

     sel->flags ^= FreeFlag;

     if(sel->flags & FreeFlag)
     {
          split_set_current(NULL, sel);
          sel->flags &= ~(TileFlag | MaxFlag | LMaxFlag);
          client_moveresize(sel, sel->free_geo, True);
          client_raise(sel);
     }
     else
     {
          sel->free_geo = sel->geo;
          sel->ogeo = sel->geo;
          split_set_current(sel, NULL);
     }

     client_update_attributes(sel);

     tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
     layout_func(selscreen, seltag[selscreen]);

     return;
}

/** Toggle the selected client to max
 * \param cmd uicb_t type unused
*/
void
uicb_togglemax(uicb_t cmd)
{
     (void)cmd;

     if(!sel || ishide(sel, selscreen) || (sel->flags & (HintFlag | FSSFlag)))
          return;

     sel->flags ^= MaxFlag;

     if(sel->flags & MaxFlag)
     {
          sel->ogeo = sel->geo;
          sel->free_geo = sel->geo;
          sel->flags &= ~(TileFlag | FreeFlag);

          split_set_current(NULL, sel);
          client_maximize(sel);
          XRaiseWindow(dpy, sel->frame);
     }
     else
     {
          sel->geo = sel->ogeo;

          client_moveresize(sel, sel->geo, True);

          tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;

          split_set_current(sel, NULL);
          layout_func(selscreen, seltag[selscreen]);
     }

     return;
}

/** Toggle the resizehint
 * \param cmd uicb_t type
 */
void
uicb_toggle_resizehint(uicb_t cmd)
{
     Client *c;

     screen_get_sel();
     (void)cmd;

     tags[selscreen][seltag[selscreen]].flags ^= ResizeHintFlag;

     for(c = tiled_client(selscreen, clients); c; c = tiled_client(selscreen, c->next))
          client_moveresize(c, c->geo, (tags[selscreen][seltag[selscreen]].flags & ResizeHintFlag));

     return;
}

/** Toggle abovefc option
  *\param cmd uicb_t type
  */
void
uicb_toggle_abovefc(uicb_t cmd)
{
     Client *c;
     (void)cmd;

     screen_get_sel();

     tags[selscreen][seltag[selscreen]].flags ^= AboveFCFlag;

     if(!(tags[selscreen][seltag[selscreen]].flags & AboveFCFlag))
     {
          for(c = clients; c; c = c->next)
               if(c->flags & AboveFlag
                         && c->screen == selscreen
                         && c->tag == (uint)seltag[selscreen])
               {
                    c->flags &= ~AboveFlag;
                    break;
               }

          tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
          layout_func(selscreen, seltag[selscreen]);
     }

     client_focus(sel);

     return;
}

/** Set the layout
 * \param cmd uicb_t type
*/
void
uicb_set_layout(uicb_t cmd)
{
     size_t i, j, n;

     screen_get_sel();

     /* Set layout_list lenght */
     for(n = 0; layout_list[n].name != NULL && layout_list[n].func != NULL; ++n);

     for(i = 0; i < n; ++i)
          if(!strcmp(cmd, xstrdup(layout_list[i].name)))
               for(j = 0; j < LEN(conf.layout); ++j)
                    if(layout_list[i].func == conf.layout[j].func)
                         tags[selscreen][seltag[selscreen]].layout = conf.layout[j];

     tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
     tags[selscreen][seltag[selscreen]].flags &= ~SplitFlag;
     arrange(selscreen, True);

     return;
}

/** Set the client as master
 * \param c Client
 */
void
layout_set_client_master(Client *c)
{
     screen_get_sel();

     if(!c || (c->flags & (HintFlag | FSSFlag)) || !(c->flags & TileFlag))
          return;

     if(c == tiled_client(selscreen, clients))
          CHECK((c = tiled_client(selscreen, c->next)));

     client_detach(c);
     client_attach(c);

     tags[selscreen][seltag[selscreen]].flags |= CleanFactFlag;
     layout_func(selscreen, seltag[selscreen]);

     return;
}

/** Check the selected client is max
 * \param cmd uicb_t type unused
*/
Bool
uicb_checkmax(uicb_t cmd)
{
     (void)cmd;

     if(!sel)
          return False;

     if(sel->flags & MaxFlag)
          return True;

     return False;
}

/** Check the selected client is free
 * \param cmd uicb_t type unused
*/
Bool
uicb_checkfree(uicb_t cmd)
{
     (void)cmd;

     if(!sel)
          return False;

     if(sel->flags & FreeFlag)
          return True;

     return False;
}

/** Check layout type
 * \param cmd uicb_t type layout type
*/
Bool
uicb_checklayout(uicb_t cmd)
{
     screen_get_sel();
     char *type = tags[selscreen][seltag[selscreen]].layout.type;

     if(type && !strcmp(cmd, type))
          return True;

     return False;
}

