/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "status.h"
#include "barwin.h"
#include "infobar.h"
#include "util.h"

void
status_manage(struct element *e)
{
     struct barwin *b = SLIST_FIRST(&e->bars);
     int l;

     barwin_refresh_color(b);

     if(e->infobar->status)
     {
          l = draw_textw(e->infobar->theme, e->infobar->status);
          draw_text(b->dr, e->infobar->theme, e->geo.w - l,
                    TEXTY(e->infobar->theme, e->geo.h), b->fg, e->infobar->status);
          barwin_refresh(b);
     }
}

/* Syntax: "<infobar name> <status string>" */
void
uicb_status(Uicb cmd)
{
     struct infobar *ib;
     struct screen *s;
     int i = 0;
     char si[128] = { 0 };

     if(!cmd)
          return;

     /* Get infobar name */
     while(cmd[i] && cmd[i] != ' ')
          si[i] = cmd[i++];
     ++i;

     SLIST_FOREACH(s, &W->h.screen, next)
     {
          SLIST_FOREACH(ib, &s->infobars, next)
               if(!strcmp(si, ib->name))
               {
                    free(ib->status);
                    ib->status = xstrdup(cmd + i);
                    infobar_elem_screen_update(s, ElemStatus);
               }
     }
}
