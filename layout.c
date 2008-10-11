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

void
freelayout(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
     {
          if(!ishide(c))
          {
               if(c->tile || c->lmax)
               {
                    moveresize(c, c->ox, c->oy, c->ow, c->oh, True);
                    c->tile = False;
                    c->lmax = False;
               }
          }
     }

     return;
}

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

void
uicb_layout_next(uicb_t cmd)
{
     layoutswitch(True);

     return;
}

void
uicb_layout_prev(uicb_t cmd)
{
     layoutswitch(False);

     return;
}

void
maxlayout(void)
{
     Client *c;

     for(c = nexttiled(clients); c; c = nexttiled(c->next))
     {
          c->tile = False;
          c->lmax = True;
          c->ox = c->x; c->oy = c->y;
          c->ow = c->w; c->oh = c->h;

          moveresize(c, 0, (conf.ttbarheight + ((conf.bartop) ? barheight : 0)),
                     (mw - (conf.borderheight * 2)),
                     (mh - (conf.borderheight * 2) - conf.ttbarheight - barheight), False);
     }

     return;
}

/* To use in a for, select only the
 * client who can be tiled */
Client*
nexttiled(Client *c)
{
     for(; c && (c->max || c->free || ishide(c)); c = c->next);

     return c;
}

void
uicb_set_mwfact(uicb_t cmd)
{
     double c;

     if(!(sscanf(cmd, "%lf", &c)))
        return;
     if(tags[seltag].mwfact + c > 0.95
        || tags[seltag].mwfact + c < 0.05
        || tags[seltag].layout.func != tile)
          return;
     tags[seltag].mwfact += c;
     arrange();

     return;
}

void
uicb_set_nmaster(uicb_t cmd)
{
     int n = atoi(cmd);

     if(tags[seltag].nmaster + n == 0)
          return;
     tags[seltag].nmaster += n;
     arrange();

     return;
}

void
tile(void)
{
     uint i, n, x, y, yt, w, h, ww, hh, th;
     uint barto, bord, mwf, nm, mht;
     Client *c;

     bord   =  conf.borderheight * 2;
     barto  =  conf.ttbarheight + barheight;
     mwf    =  tags[seltag].mwfact * mw;
     nm     =  tags[seltag].nmaster;
     mht    =  mh - ((conf.bartop) ? 0 : barheight);

     /* count all the "can-be-tiled" client */
     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     if(!n)
          return;

     /* window geoms */
     hh = ((n <= nm) ? mht / (n > 0 ? n : 1) : mht / nm) - bord*2;
     ww = (n  <= nm) ? mw : mwf;
     th = (n  >  nm) ? mht / (n - nm) : 0;
     if(n > nm && th < barheight)
          th = mht;

     x = 0;
     y = yt = barto;

     if(!conf.bartop)
          y = yt = conf.ttbarheight;

     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), i++)
     {
          c->max = False;
          c->lmax = False;
          c->tile = True;
          c->ox = c->x; c->oy = c->y;
          c->ow = c->w; c->oh = c->h;

          /* MASTER CLIENT */
          if(i < nm)
          {
               y = yt + i * hh;
               w = ww - bord;
               h = hh;
               /* remainder */
               if(i + 1 == (n < nm ? n : nm))
                    h = (mht - hh*i) -
                         ((conf.bartop) ? barheight: 0);
               h -= bord + conf.ttbarheight;
          }
          /* TILE CLIENT */
          else
          {
               if(i == nm)
               {
                    y = yt;
                    x += ww;
               }
               w = mw - ww - bord;
               /* remainder */
               if(i + 1 == n)
                    h = (barto + mht) - y - (bord + barto);
               else
                    h = th - (bord + conf.ttbarheight) - bord*2;
          }
          moveresize(c, x, y, w, h, tags[seltag].resizehint);
          if(n > nm && th != mht)
               y = c->y + c->h + bord + conf.ttbarheight;
     }

     return;
}

void
uicb_tile_switch(uicb_t cmd)
{
     Client *c;

     if(!sel || sel->hint || !sel->tile)
          return;
     if((c = sel) == nexttiled(clients))
          if(!(c = nexttiled(c->next)))
               return;
     detach(c);
     attach(c);
     focus(c);
     arrange();

     return;
}

void
uicb_togglemax(uicb_t cmd)
{
     if(!sel || ishide(sel) || sel->hint)
          return;
     if(!sel->max)
     {
          sel->ox = sel->x; sel->oy = sel->y;
          sel->ow = sel->w; sel->oh = sel->h;
          moveresize(sel, 0, (conf.ttbarheight + ((conf.bartop) ? barheight: 0)),
                     (mw - (conf.borderheight * 2)),
                     (mh - (conf.borderheight * 2)- conf.ttbarheight - barheight), False);
          raiseclient(sel);
          sel->max = True;
     }
     else if(sel->max)
     {
          moveresize(sel, sel->ox, sel->oy, sel->ow, sel->oh, False);
          sel->max = False;
     }
     arrange();

     return;
}

