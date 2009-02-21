/*
*      launcher.c
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
launcher_execute(Launcher launcher)
{
     XEvent ev;
     KeySym ks;
     char tmp[32] = { 0 };
     char buf[512] = { 0 };
     int pos = 0;
     Bool run = True;

     int x = (infobar[selscreen].layout_button->geo.x
              + textw(tags[selscreen][seltag[selscreen]].layout.symbol) + PAD);

     XGrabKeyboard(dpy, ROOT, True, GrabModeAsync, GrabModeAsync, CurrentTime);

     barwin_draw_text(infobar[selscreen].bar, x, font->height, launcher.prompt);

     while(run)
     {
          if(ev.type == KeyPress)
          {
               XLookupString(&ev.xkey, tmp, sizeof(tmp), &ks, 0);

               /* Check Ctrl-c / Ctrl-d */
               if(ev.xkey.state & ControlMask)
                    if(ks == XK_c
                       || ks == XK_d)
                         ks = XK_Escape;
               switch(ks)
               {
               case XK_Return:
                    spawn("%s %s", launcher.command, buf);
                    run = 0;
                    break;
               case XK_Escape:
                    run = 0;
                    break;
               case XK_BackSpace:
                    if(pos)
                         buf[--pos] = 0;
                    break;
               default:
                    strncat(buf, tmp, sizeof(buf));
                    ++pos;
                    break;
               }

               barwin_refresh_color(infobar[selscreen].bar);
               barwin_draw_text(infobar[selscreen].bar, x, font->height, launcher.prompt);
               barwin_draw_text(infobar[selscreen].bar, x + textw(launcher.prompt) + textw(" "), font->height, buf);
               barwin_refresh(infobar[selscreen].bar);
          }
          else
               getevent(ev);
          XNextEvent(dpy, &ev);
     }

     infobar_draw(selscreen);

     XUngrabKeyboard(dpy, CurrentTime);

     return;

}

void
uicb_launcher(uicb_t cmd)
{
     int i;

     for(i = 0; i < conf.nlauncher; ++i)
          if(!strcmp(cmd, conf.launcher[i].name))
               launcher_execute(conf.launcher[i]);

     return;
}
