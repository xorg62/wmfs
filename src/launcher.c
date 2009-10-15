/*
*      launcher.c
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

/* conforming to glib use _GNU_SOURCE for asprintf declaration */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "wmfs.h"


static char *complete_on_command(char*, size_t);
static char *complete_on_files(char*, size_t);

void
launcher_execute(Launcher launcher)
{
     BarWindow *bw;
     Bool found;
     Bool lastwastab = False;
     Bool my_guitar_gently_wheeps = True;
     char tmp[32] = { 0 };
     char buf[512] = { 0 };
     char tmpbuf[512] = { 0 };
     char *complete;
     int pos = 0, x;
     int tabhits = 0;
     KeySym ks;
     XEvent ev;

     screen_get_sel();

     x = (infobar[selscreen].layout_button->geo.x
          + textw(tags[selscreen][seltag[selscreen]].layout.symbol) + PAD);

     XGrabKeyboard(dpy, ROOT, True, GrabModeAsync, GrabModeAsync, CurrentTime);

     bw = barwin_create(infobar[selscreen].bar->win, x, 1,
                        infobar[selscreen].bar->geo.width - x - 1,
                        infobar[selscreen].bar->geo.height - 2,
                        infobar[selscreen].bar->bg,
                        infobar[selscreen].bar->fg,
                        False, False, conf.border.bar);

     barwin_map(bw);
     barwin_refresh_color(bw);

     /* First draw of the cursor */
     XSetForeground(dpy, gc, getcolor(infobar[selscreen].bar->fg));
     XDrawLine(dpy, bw->dr, gc, textw(launcher.prompt) + textw(" "),
               2, textw(launcher.prompt) + textw(" "), INFOBARH - 4);

     barwin_refresh(bw);

     barwin_draw_text(bw, 1, FHINFOBAR - 1, launcher.prompt);

     while(my_guitar_gently_wheeps)
     {
          if(ev.type == KeyPress)
          {
               XLookupString(&ev.xkey, tmp, sizeof(tmp), &ks, 0);

               /* Check Ctrl-c / Ctrl-d */
               if(ev.xkey.state & ControlMask)
                    if(ks == XK_c || ks == XK_d)
                         ks = XK_Escape;

               /* Check if there is a keypad */
               if(IsKeypadKey(ks) && ks == XK_KP_Enter)
                    ks = XK_Return;

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
                    /*
                     * completion
                     * if there is not space in buffer we
                     * complete the command using complete_on_command.
                     * Else we try to complete on filename using
                     * complete_on_files.
                     */
                    buf[pos] = '\0';
                    if (lastwastab)
                         tabhits++;
                    else
                    {
                         tabhits = 1;
                         strcpy(tmpbuf, buf);
                    }


                    if (pos)
                    {
                         if (strchr(tmpbuf, ' '))
                              complete = complete_on_files(tmpbuf, tabhits);
                         else
                              complete = complete_on_command(tmpbuf, tabhits);

                         if (complete)
                         {
                              strcpy(buf, tmpbuf);
                              strncat(buf, complete, sizeof(buf));
                              found = True;
                              free(complete);
                         }
                    }

                    lastwastab = True;

                    /* start a new round of tabbing */
                    if (!found)
                         tabhits = 0;

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

               /* Update cursor position */
               XSetForeground(dpy, gc, getcolor(infobar[selscreen].bar->fg));
               XDrawLine(dpy, bw->dr, gc,
                         1 + textw(launcher.prompt) + textw(" ") + textw(buf), 2,
                         1 + textw(launcher.prompt) + textw(" ") + textw(buf), INFOBARH - 4);

               barwin_draw_text(bw, 1, FHINFOBAR - 1, launcher.prompt);
               barwin_draw_text(bw, 1 + textw(launcher.prompt) + textw(" "), FHINFOBAR - 1, buf);
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

/*
 * Just search command in PATH.
 * Return the characters to complete the command.
 */
static char *
complete_on_command(char *start, size_t hits)
{
     char *path;
     char *dirname;
     char *ret = NULL;
     DIR *dir;
     struct dirent *content;
     size_t count = 0;

     if (!getenv("PATH") || !start || hits <= 0)
         return NULL;

     path = _strdup(getenv("PATH"));
     dirname = strtok(path, ":");

     /* recursively open PATH */
     while (dirname)
     {
          if ((dir = opendir(dirname)))
          {
               while ((content = readdir(dir)))
                    if (!strncmp(content->d_name, start, strlen(start)) && ++count == hits)
                    {
                         ret = _strdup(content->d_name + strlen(start));
                         break;
                    }
               closedir(dir);
          }

          if (ret)
              break;

          dirname = strtok(NULL, ":");
     }

     free(path);

     return ret;
}

/*
 * Complete a filename or directory name.
 * works like complete_on_command.
 */
static char *
complete_on_files(char *start, size_t hits)
{
     char *ret = NULL;
     char *p = NULL;
     char *dirname = NULL;
     char *path = NULL;
     char *filepath = NULL;
     DIR *dir = NULL;
     struct dirent *content = NULL;
     struct stat st;
     size_t count = 0;

     if (!start || hits <= 0 || !(p = strrchr(start, ' ')))
          return NULL;

     /*
      * Search the directory to open and set
      * the beginning of file to complete on pointer 'p'.
      */
     if (*(++p) == '\0' || !strrchr(p, '/'))
          path = _strdup(".");
     else
     {
          /* remplace ~ by $HOME in dirname */
          if (!strncmp(p, "~/", 2) && getenv("HOME"))
               asprintf(&dirname, "%s%s", getenv("HOME"), p+1);
          else
               dirname = _strdup(p);

          /* Set p to filename to be complete
           * and path the directory containing the file
           * /foooooo/baaaaaar/somethinglikethis<tab>
           * <---- path - ---><------- p ------>
           */
          p = strrchr(dirname, '/');
          if (p != dirname)
          {
               *(p++) = '\0';
               path = _strdup(dirname);
          }
          else
          {
               path = _strdup("/");
               p++;
          }
     }

     if ((dir = opendir(path)))
     {
          while ((content = readdir(dir)))
          {
               if (!strcmp(content->d_name, ".") || !strcmp(content->d_name, ".."))
                    continue;
               if (!strncmp(content->d_name, p, strlen(p)) && ++count == hits)
               {
                    /* If it's a directory append '/' to the completion */
                    asprintf(&filepath, "%s/%s", path, content->d_name);

                    if (filepath && stat(filepath, &st) != -1)
                    {
                         if (S_ISDIR(st.st_mode))
                              asprintf(&ret, "%s/", content->d_name + strlen(p));
                         else
                              ret = _strdup(content->d_name + strlen(p));
                    }
                    else
                         warn("%s", filepath);

                    IFREE(filepath);

                    break;
               }
          }
          closedir(dir);
     }

     IFREE(dirname);
     IFREE(path);

     return ret;
}
