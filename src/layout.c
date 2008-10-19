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
arrange(void)
{
     Client *c;

     for(c = clients; c; c = c->next)
          if(!ishide(c))
               client_unhide(c);
          else
               client_hide(c);

     tags[seltag].layout.func();

     if(selbytag[seltag])
          client_focus(selbytag[seltag]);
     else
          client_focus(NULL);

     updatebar();

     return;
}

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
                    geo.x = c->ogeo.x; geo.y = c->ogeo.y;
                    geo.width = c->ogeo.width;
                    geo.height = c->ogeo.height;
                    client_moveresize(c, geo, True);
                    c->tile = c->lmax = False;
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
     XRectangle geo;

     for(c = nexttiled(clients); c; c = nexttiled(c->next))
     {
          c->tile = False;
          c->lmax = True;
          c->ogeo.x = c->geo.x; c->ogeo.y = c->geo.y;
          c->ogeo.width = c->geo.width; c->ogeo.height = c->geo.height;

          geo.x = sgeo.x; geo.y = sgeo.y;
          geo.width = sgeo.width - (conf.borderheight * 2);
          geo.height = sgeo.height - (conf.borderheight * 2);

          client_moveresize(c, geo, False);
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
     Client *c;
     XRectangle mastergeo = {0, 0, 0, 0};
     XRectangle cgeo = {sgeo.x, sgeo.y, 0, 0};
     uint n, mwfact = tags[seltag].mwfact * sgeo.width;
     uint nmaster = tags[seltag].nmaster;
     uint tileheight, i, border = conf.borderheight*2;

     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     if(!n)
          return;

     /* Define the master(s) client(s) size */
     if(n <= nmaster)
     {
          mastergeo.height = sgeo.height / (n > 0 ? n : 1);
          mastergeo.width = sgeo.width;
     }
     else
     {
          mastergeo.height = sgeo.height / nmaster;
          mastergeo.width = mwfact;
     }

     /* Define the tiled clients size, so if the clients number > nmaster */
     if(n > nmaster)
          tileheight = sgeo.height / (n - nmaster);
     else
          tileheight = 0;

     if(n > nmaster && tileheight < barheight)
          tileheight = sgeo.height;


     for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next), i++)
     {
          /* Set client property */
          c->max = c->lmax = False;
          c->tile = True;
          c->ogeo.x = c->geo.x; c->ogeo.y = c->geo.y;
          c->ogeo.width = c->geo.width; c->ogeo.height = c->geo.height;

          /* Master Client */
          if(i < nmaster)
          {
               cgeo.y = sgeo.y + (i * mastergeo.height);
               cgeo.width = mastergeo.width - border;
               cgeo.height = mastergeo.height;

               /* Remainder */
               if(i + 1 == (n < nmaster ? n : nmaster))
                    cgeo.height = (sgeo.height - mastergeo.height * i);

               cgeo.height -= border;
          }

          /* Tiled Client */
          else
          {
               if(i == nmaster)
               {
                    cgeo.y = sgeo.y;
                    cgeo.x += mastergeo.width;
               }

               cgeo.width = sgeo.width - mastergeo.width - border;

               /* Remainder */
               if(i + 1 == n)
                    cgeo.height = (sgeo.y + sgeo.height) - cgeo.y - border;
               else
                    cgeo.height = tileheight - border;
          }

          //cgeo.height -= conf.ttbarheight;

          client_moveresize(c, cgeo, tags[seltag].resizehint);

          if(n > nmaster && tileheight != sgeo.height)
               cgeo.y = c->geo.y + c->geo.height + border + conf.ttbarheight;
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
     client_detach(c);
     client_attach(c);
     client_focus(c);
     arrange();

     return;
}

void
uicb_togglemax(uicb_t cmd)
{
     XRectangle geo;

     if(!sel || ishide(sel) || sel->hint)
          return;
     if(!sel->max)
     {
          sel->ogeo.x = sel->geo.x; sel->ogeo.y = sel->geo.y;
          sel->ogeo.width = sel->geo.width; sel->ogeo.height = sel->geo.height;

          geo.x = sgeo.x; geo.y = sgeo.y;
          geo.width = sgeo.width - (conf.borderheight * 2);
          geo.height = sgeo.height - (conf.borderheight * 2);

          client_moveresize(sel, geo, False);
          raiseclient(sel);
          sel->max = True;
     }
     else if(sel->max)
     {
          geo.x = sel->ogeo.x; geo.y = sel->ogeo.y;
          geo.width = sel->ogeo.width; geo.height = sel->ogeo.height;

          client_moveresize(sel, geo, False);
          sel->max = False;
     }
     arrange();

     return;
}


