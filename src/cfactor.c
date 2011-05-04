/*
*      cfactor.c
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

#define RPOS(x) (x % 2 ? p - 1 : p + 1)

char scanfac[4][2] =
{
     { 1,  0 }, { -1, 0 }, /* Right, Left   */
     { 0, -1 }, {  0, 1 }  /* Top,   Bottom */
};

/** Clean client tile factors
  *\param c Client pointer
*/
void
cfactor_clean(Client *c)
{
     CHECK(c);

     if(!tags[c->screen][c->tag].cleanfact)
          return;

     c->tilefact[Right] = c->tilefact[Left]   = 0;
     c->tilefact[Top]   = c->tilefact[Bottom] = 0;

     return;
}

/** Return new geo of client with factors applied
  *\param c Client pointer
  *\return geo
*/
XRectangle
cfactor_geo(XRectangle geo, int fact[4])
{
     XRectangle cgeo = { 0, 0, 0, 0 };

     cgeo = geo;

     /* Right factor */
     cgeo.width += fact[Right];

     /* Left factor */
     cgeo.x -= fact[Left];
     cgeo.width += fact[Left];

     /* Top factor */
     cgeo.y -= fact[Top];
     cgeo.height += fact[Top];

     /* Bottom factor */
     cgeo.height += fact[Bottom];

     return cgeo;
}

static Bool
cfactor_parentrow(Client *c, Client *cc, Position p)
{
     Bool ret;

     if(!c || !cc)
          return False;

     switch(p)
     {
          default:
          case Right:
               ret = (cc->frame_geo.x + cc->frame_geo.width == c->frame_geo.x + c->frame_geo.width);
               break;
          case Left:
               ret = (cc->frame_geo.x == c->frame_geo.x);
               break;
          case Top:
               ret = (cc->frame_geo.y == c->frame_geo.y);
               break;
          case Bottom:
               ret = (cc->frame_geo.y + cc->frame_geo.height == c->frame_geo.y + c->frame_geo.height);
               break;
     }

      return ret;
}

static void
cfactor_scan_row(Client *c, Position p, int fac)
{
     Client *cc;

     for(cc = tiled_client(c->screen, clients); cc; cc = tiled_client(c->screen, cc->next))
     {
          if(cc == c)
               continue;

          if(cfactor_parentrow(c, cc, p))
          {
               cc->tilefact[p] += fac;
               client_moveresize(cc, cc->geo,  tags[cc->screen][cc->tag].resizehint);
          }
     }

     return;
}

/** Manual resizing of tiled clients
  * \param c Client pointer
  * \param p Direction of resizing
  * \param fac Factor of resizing
*/
static void
cfactor_set(Client *c, Position p, int fac)
{
     Client *gc = NULL;
     int x, y;
     XRectangle cgeo, scgeo;
     int cfact[4] = { 0 }, scfact[4] = { 0 };

     if(!c || p > Bottom)
          return;

     /* Start place of pointer for faster scanning */
     x = c->geo.x + ((p == Right)  ? c->geo.width  : 0);
     y = c->geo.y + ((p == Bottom) ? c->geo.height : 0);
     y += ((p < Top)  ? c->geo.height / 2 : 0);
     x += ((p > Left) ? c->geo.width  / 2 : 0);

     /* Scan in right direction to next(p) physical client */
     for(; (gc = client_gb_pos(c, x, y)) == c; x += scanfac[p][0], y += scanfac[p][1]);

     if(!gc || c->screen != gc->screen)
          return;

     cfact[p] += fac;
     scfact[RPOS(p)] -= fac;

     cgeo  = cfactor_geo(c->geo,  cfact);
     scgeo = cfactor_geo(gc->geo, scfact);

     /* Too big */
     if(scgeo.width > (1 << 15) || scgeo.height > (1 << 15)
          || cgeo.width > (1 << 15) || cgeo.height > (1 << 15))
          return;

     /* Too small */
     if(scgeo.width < 1 || scgeo.height < 1
          || cgeo.width < 1 || cgeo.height < 1)
          return;

     c->tilefact[p] += fac;
     gc->tilefact[RPOS(p)] -= fac;

     /* Arrange row parents */
     cfactor_scan_row(c, p, fac);
     cfactor_scan_row(gc, RPOS(p), -fac);

     /* Magic moment */
     client_moveresize(c,  cgeo,  tags[c->screen][c->tag].resizehint);
     client_moveresize(gc, scgeo, tags[gc->screen][gc->tag].resizehint);

     return;
}


void
uicb_client_resize_right(uicb_t cmd)
{
     int n = atoi(cmd);

     CHECK(sel);

     cfactor_set(sel, Right, n);

     return;
}

void
uicb_client_resize_left(uicb_t cmd)
{
     int n = atoi(cmd);

     CHECK(sel);

     cfactor_set(sel, Left, n);

     return;
}

void
uicb_client_resize_top(uicb_t cmd)
{
     int n = atoi(cmd);

     CHECK(sel);

     cfactor_set(sel, Top, n);

     return;
}

void
uicb_client_resize_bottom(uicb_t cmd)
{
     int n = atoi(cmd);

     CHECK(sel);

     cfactor_set(sel, Bottom, n);

     return;
}

