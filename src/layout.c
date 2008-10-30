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

     if(selbytag[seltag] != NULL
         && selbytag[seltag]->tbar != NULL)
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
          geo.width = sgeo.width - (c->border * 2);
          geo.height = sgeo.height - (c->border * 2);

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
        || tags[seltag].layout.func == maxlayout
        || tags[seltag].layout.func == freelayout)
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
multi_tile(Position type)
{
     Client *c;
     XRectangle mastergeo = {sgeo.x, sgeo.y, 0, 0};
     XRectangle cgeo = {sgeo.x, sgeo.y, 0, 0};
     uint n, mwfact, nmaster = tags[seltag].nmaster;
     uint tilesize = 0;
     int i, border = conf.client.borderheight * 2;

     for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next), ++n);
     if(!n || n < nmaster)
          return;

     /* SET MWFACT */
     mwfact = (type == Top || type == Bottom)
          ? tags[seltag].mwfact * sgeo.height
          : tags[seltag].mwfact * sgeo.width;

     /* MASTER SIZE */
     if(type == Top || type == Bottom)
     {
          if(type == Top)
               mastergeo.y = (n <= nmaster) ? sgeo.y : sgeo.y + (sgeo.height - mwfact) - border;
          mastergeo.width = (sgeo.width / nmaster) - border;
          mastergeo.height = (n <= nmaster) ? sgeo.height - border : mwfact;
     }
     else
     {
          if(type == Left)
               mastergeo.x = (n <= nmaster) ? sgeo.x : sgeo.width - mwfact - border;
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
                    cgeo.height -= (conf.titlebar.height + border);
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
                         cgeo.y += mastergeo.height + conf.titlebar.height + border;
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
                    cgeo.height = sgeo.height - mastergeo.height - conf.titlebar.height - border*2;
               }
               else
               {
                    cgeo.width = sgeo.width - mastergeo.width - border*2;
                    cgeo.height = tilesize;
                    cgeo.height -= border + conf.titlebar.height;
               }
          }

          /* REMAINDER */
          if(i + 1 == n  || i + 1 == (n < nmaster ? n : nmaster))
          {
               if(type == Top || type == Bottom)
                    cgeo.width = sgeo.width - cgeo.x - border;
               else
                    cgeo.height = (sgeo.y + sgeo.height) - cgeo.y - border;
          }

          /* Magic instant */
          client_moveresize(c, cgeo, False);

          /* Set the position of the next client */
          if(type == Top || type == Bottom)
               cgeo.x = c->geo.x + c->geo.width + border;
          else
               cgeo.y = c->geo.y + c->geo.height + border + conf.titlebar.height;
     }

     return;
}

void
tile(void)
{
     multi_tile(Right);

     return;
}

void
tile_left(void)
{
     multi_tile(Left);

     return;
}

void
tile_top(void)
{
     multi_tile(Top);

     return;
}
void
tile_bottom(void)
{
     multi_tile(Bottom);

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
          geo.width = sgeo.width - (sel->border * 2);
          geo.height = sgeo.height - (sel->border * 2);

          client_moveresize(sel, geo, False);
          client_raise(sel);
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


