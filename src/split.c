/*
*      split.c
*      Copyright © 2011 Martin Duquesnoy <xorg62@gmail.com>
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

/** Arrange size of parent client of last closed client
*/
static void
_split_arrange_size(XRectangle g, XRectangle *cg, Position p)
{
     if(p < Top)
          cg->width += FRAMEW(g.width);
     else
          cg->height += FRAMEH(g.height);

     if(p == Right)
          cg->x -= FRAMEW(g.width);

     if(p == Bottom)
          cg->y -= FRAMEH(g.height);

     return;
}

/** Check if parent client of last closed client is OK for row resize
*/
static Bool
_split_check_row(XRectangle g1, XRectangle g2, Position p)
{
     if(p < Top)
          return (g1.y >= g2.y && (g1.y + g1.height) <= (g2.y + g2.height));
     else
          return (g1.x >= g2.x && (g1.x + g1.width) <= (g2.x + g2.width));
}

/** Arrange clients after a client close
  *\param ghost Ghost client
 */
void
split_arrange_closed(Client *ghost)
{
     Position p;
     Bool b = False;
     XRectangle cgeo;
     Client *c, *cc;
     int screen = ghost->screen;

     if(tags[screen][seltag[screen]].layout.func != split)
          return;

     /* Use ghost client to fix holes in tile
      *     .--.  ~   ~
      *    /xx  \   ~   ~
      *  ~~\O _ (____     ~
      *  __.|    .--'-==~   ~
      * '---\    '.      ~  ,  ~
      *      '.    '-.___.-'/   ~
      *        '-.__     _.'  ~
      *             `````   ~
      */

     /* Search for single parent for easy resize
      * Example case:
      *  ___________               ___________
      * |     |  B  | ->       -> |     |     |
      * |  A  |_____| -> Close -> |  A  |  B  |
      * |     |  C  | ->   C   -> |     |v v v|
      * |_____|_____| ->       -> |_____|_____|
      */
     for(p = Right; p < Bottom + 1; ++p)
          if((c = client_get_next_with_direction(ghost, p)))
               if(cfactor_check_2pc(ghost->frame_geo, c->frame_geo, p))
               {
                    _split_arrange_size(ghost->wrgeo, &c->wrgeo, p);
                    cfactor_clean(c);
                    client_moveresize(c, (c->pgeo = c->wrgeo), tags[screen][c->tag].resizehint);

                    return;
               }

     /* Check row parents for full resize
      * Example case:
      *  ___________               ___________
      * |     |  B  | ->       -> | <<  B     |
      * |  A  |_____| -> Close -> |___________|
      * |     |  C  | ->   A   -> | <<  C     |
      * |_____|_____| ->       -> |___________|
      */
     for(p = Right; p < Bottom + 1 && !b; ++p)
          if((c = client_get_next_with_direction(ghost, p)))
               for(cgeo = c->frame_geo, cc = tiled_client(c->screen, clients);
                         cc; cc = tiled_client(c->screen, cc->next))
               {
                    if(cfactor_parentrow(cgeo, cc->frame_geo, RPOS(p))
                              && _split_check_row(cc->frame_geo, ghost->frame_geo, p))
                    {
                         _split_arrange_size(ghost->wrgeo, &cc->wrgeo, p);
                         cfactor_clean(cc);
                         client_moveresize(cc, (cc->pgeo = cc->wrgeo), tags[screen][cc->tag].resizehint);
                         b = True;
                    }
               }

     return;
}

/** Split client hor or vert to insert another client in the new area
  *\param c Client pointer
  *\param p True = Vertical, False = Horizontal
  *\return sgeo Geo of future integrated client
*/
XRectangle
split_client(Client *c, Bool p)
{
     XRectangle geo, sgeo;

     if(!c || !(c->flags & TileFlag)
               || tags[c->screen][c->tag].layout.func != split)
          return c->wrgeo;

     cfactor_clean(c);

     c->flags &= ~(MaxFlag | LMaxFlag);
     c->flags |= (TileFlag | SplitFlag);

     /* Use geometry without resizehint applied on it */
     geo = sgeo = c->wrgeo;

     /* Vertical */
     if(p)
     {
          geo.width /= 2;
          sgeo.x = FRAMEW(geo.x + geo.width);
          sgeo.width = (sgeo.width / 2) - (BORDH * 2);

          /* Remainder */
          sgeo.width += (c->wrgeo.x + c->wrgeo.width) - (sgeo.x + sgeo.width);
      }
     /* Horizontal */
     else
     {
          geo.height = (geo.height / 2) - TBARH;
          sgeo.y = FRAMEH(geo.y + geo.height);
          sgeo.height = (sgeo.height / 2) - BORDH;

          /* Remainder */
          sgeo.height += (c->wrgeo.y + c->wrgeo.height) - (sgeo.y + sgeo.height);
     }

     client_moveresize(c, (c->pgeo = geo), tags[c->screen][c->tag].resizehint);

     return sgeo;
}

/** Apply new attributes to splitted client
  *\param c Client pointer
  *\param geo New geo
*/
void
split_client_fill(Client *c, XRectangle geo)
{
     if(!c || tags[c->screen][c->tag].layout.func != split)
          return;

     c->flags &= ~(MaxFlag | LMaxFlag);
     c->flags |= (TileFlag | SplitFlag);

     cfactor_clean(c);
     client_moveresize(c, (c->pgeo = geo), tags[c->screen][c->tag].resizehint);

     return;
}

/** Integrate client in tag
  *\param c Client pointer (integrate)
  *\param sc Splitted client pointer
*/
void
split_client_integrate(Client *c, Client *sc, int screen, int tag)
{
     XRectangle g;

     if(tags[screen][tag].layout.func != split
               || c->flags & FreeFlag
               || !tags[screen][tag].nclients || !c)
          return;

     if(!sc || sc->screen != screen || sc->tag != tag)
          if(!(sc = tiled_client(screen, clients)))
               return;

     g = split_client(sc, (sc->frame_geo.height < sc->frame_geo.width));
     split_client_fill(c, g);

     return;
}


