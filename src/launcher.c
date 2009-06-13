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
#include <dirent.h>
#include <limits.h>

void
launcher_execute(Launcher launcher)
{
     XEvent ev;
     KeySym ks;
     char tmp[32] = { 0 };
     char buf[512] = { 0 };
     char tabbuf[512] = { 0 };
     int pos = 0;
     DIR *dir;
     struct dirent *dirent;
     char *searchpath;
     char *start, *end;
     Bool stop, found;
     char currentpath[PATH_MAX];
     Bool lastwastab = False;
     int tabhits = 0;
     int searchhits = 0;

     BarWindow *bw;
     Bool my_guitar_gently_wheeps = True;

     screen_get_sel();

     int x = (infobar[selscreen].layout_button->geo.x
              + textw(tags[selscreen][seltag[selscreen]].layout.symbol) + PAD);

     XGrabKeyboard(dpy, ROOT, True, GrabModeAsync, GrabModeAsync, CurrentTime);


     bw = barwin_create(infobar[selscreen].bar->win, x, 0,
                        infobar[selscreen].bar->geo.width - x,
                        infobar[selscreen].bar->geo.height - 1,
                        infobar[selscreen].bar->bg,
                        infobar[selscreen].bar->fg,
                        False, False, conf.border.bar);

     barwin_map(bw);
     barwin_refresh_color(bw);
     barwin_refresh(bw);

     barwin_draw_text(bw, 1, font->height, launcher.prompt);

     while(my_guitar_gently_wheeps)
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
                    my_guitar_gently_wheeps = 0;
                    break;
               case XK_Escape:
                    my_guitar_gently_wheeps = 0;
                    break;
               case XK_Tab:
                    stop = found = False;

                    searchpath = getenv("PATH");
                    start = searchpath;

                    if (lastwastab)
                    {
                         strcpy(buf, tabbuf);
                         tabhits++;
                    }
                    else
                         tabhits = 1;

                    searchhits = 0;

                    do
                    {
                         end = strchr(start, ':');
                         if (end == NULL)
                         {
                              stop = True;
                              strncpy(currentpath, start, PATH_MAX);
                         }
                         else
                         {
                              strncpy(currentpath, start, end - start);
                              currentpath[end - start] = '\0';
                         }
                         if (!stop)
                              start = end + 1;

                         dir = opendir(currentpath);
                         if (dir)
                         {
                              while ((dirent = readdir(dir)) != NULL)
                              {
                                   if (!strncmp(dirent->d_name, buf, strlen(buf)))
                                   {
                                        searchhits++;
                                        if (searchhits == tabhits)
                                        {
                                             strcpy(tabbuf, buf);
                                             strcpy(buf, dirent->d_name);
                                             pos = strlen(dirent->d_name);
                                             buf[pos] = '\0';
                                             found = stop = True;
                                        }
                                   }
                              }

                              closedir(dir);
                         }
                    }
                    while (!stop);

                    lastwastab = True;

                    if (!found)
                         tabhits = 0; /* start a new round of tabbing */

                    pos = strlen(buf);

                    break;

               case XK_BackSpace:
                    lastwastab = False;
                    if(pos)
                         buf[--pos] = 0;
                    break;
               default:
                    lastwastab = False;
                    strncat(buf, tmp, sizeof(buf));
                    ++pos;
                    break;
               }

               barwin_refresh_color(bw);
               barwin_draw_text(bw, 1, font->height, launcher.prompt);
               barwin_draw_text(bw, 1 + textw(launcher.prompt) + textw(" "), font->height, buf);
               barwin_refresh(bw);
          }
          else
               getevent(ev);
          XNextEvent(dpy, &ev);
     }

     barwin_unmap(bw);
     barwin_delete(bw);
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
