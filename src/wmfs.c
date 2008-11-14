/*
*      wmfs.c
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

int
errorhandler(Display *d, XErrorEvent *event)
{
     char mess[256];

     /* Check if there are another WM running */
     if(BadAccess == event->error_code
        && DefaultRootWindow(dpy) == event->resourceid)
     {
          fprintf(stderr, "WMFS Error: Another Window Manager is already running.\n");
          exit(EXIT_FAILURE);
     }

     XGetErrorText(d, event->error_code, mess, 128);
     fprintf(stderr, "WMFS error: %s(%d) opcodes %d/%d\n  resource #%lx\n", mess,
             event->error_code,
             event->request_code,
             event->minor_code,
             event->resourceid);


     return 1;
}

int
errorhandlerdummy(Display *d, XErrorEvent *event)
{
     return 0;
}

/** Clean wmfs before the exit
 */
void
quit(void)
{
     /* Exiting WMFS :'( */
     XftFontClose(dpy, font);
     XFreeCursor(dpy, cursor[CurNormal]);
     XFreeCursor(dpy, cursor[CurMove]);
     XFreeCursor(dpy, cursor[CurResize]);
     infobar_destroy();
     efree(keys);
     efree(conf.titlebar.mouse);
     efree(conf.client.mouse);
     efree(conf.root.mouse);
     XSync(dpy, False);

     return;
}

/** WMFS main loop: Check stdin and
 *  execute the event loop
 */
void
mainloop(void)
{
     fd_set fd;
     char sbuf[sizeof infobar->statustext], *p;
     int len, r, offset = 0;
     Bool readstdin = True;
     XEvent ev;

     len = sizeof infobar->statustext - 1;
     sbuf[len] = infobar->statustext[len] = '\0';

     while(!exiting)
     {
          FD_ZERO(&fd);
          if(readstdin)
               FD_SET(STDIN_FILENO, &fd);
          FD_SET(ConnectionNumber(dpy), &fd);
          if(select(ConnectionNumber(dpy) + 1, &fd, NULL, NULL, NULL) == -1)
               fprintf(stderr, "WMFS Warning: Select failed\n");
          if(FD_ISSET(STDIN_FILENO, &fd))
          {
               if((r = read(STDIN_FILENO, sbuf + offset, len - offset)))
               {
                    for(p = sbuf + offset; r > 0; ++p, --r, ++offset)
                    {
                         if(*p == '\n')
                         {
                              *p = '\0';
                              strncpy(infobar->statustext, sbuf, len);
                              p += r - 1;
                              for(r = 0; *(p - r) && *(p - r) != '\n'; ++r);
                              offset = r;
                              if(r)
                                   memmove(sbuf, p - r + 1, r);
                              break;
                         }
                    }
               }
              else
               {
                    strncpy(infobar->statustext, sbuf, strlen(sbuf));
                    readstdin = False;
               }
               infobar_draw();
          }
          while(XPending(dpy))
          {
               XNextEvent(dpy, &ev);
               getevent(ev);
          }
     }

     return;
}


/** Set the exiting variable to True
 *  for stop the main loop
 * \param cmd unused uicb_t
 */
void
uicb_quit(uicb_t cmd)
{
     exiting = True;

     return;
}

/** Scan if there are window on X
 *  for manage it
*/
void
scan(void)
{
     uint i, num;
     Window *wins = NULL, d;
     XWindowAttributes wa;

     if(XQueryTree(dpy, root, &d, &d, &wins, &num))
     {
          for(i = 0; i < num; i++)
          {
               if(!XGetWindowAttributes(dpy, wins[i], &wa)
                  || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d))
                    continue;
               if(wa.map_state == IsViewable || getwinstate(wins[i]) == IconicState)
                    client_manage(wins[i], &wa);

          }
          for(i = 0; i < num; i++)
          {
               if(!XGetWindowAttributes(dpy, wins[i], &wa))
                    continue;
               if(XGetTransientForHint(dpy, wins[i], &d)
                  && (wa.map_state == IsViewable || getwinstate(wins[i]) == IconicState))
                    client_manage(wins[i], &wa);
          }
     }
     XFree(wins);

     arrange();

     return;
}

/** Signal handle function
 * \param signum Signal number
*/
void
handle_signal(int signum)
{
     Client *c;

     if(signum == SIGTERM || signum == SIGINT)
     {
          XSetErrorHandler(errorhandlerdummy);
          for(c = clients; c; c = c->next)
          {
               XReparentWindow(dpy, c->win, root, c->frame_geo.x, c->frame_geo.y);
               client_unmanage(c);
          }
          fprintf(stderr, "\nExit WMFS... Bye !!\n");
          quit();
          exit(EXIT_FAILURE);
     }

     return;
}

/** main function
 * \param argc ?
 * \param argv ?
 * \return 0
*/
int
main(int argc, char **argv)
{
     int i;
     struct sigaction sig;

     static struct option long_options[] = {

          {"help",	 	0, NULL, 'h'},
          {"info",		0, NULL, 'i'},
          {"version",     	0, NULL, 'v'},
          {NULL,		0, NULL, 0}
     };

     while ((i = getopt_long(argc, argv, "hvi", long_options, NULL)) != -1)
     {
          switch (i)
          {
          case 'h':
          default:
               printf("Usage: wmfs [OPTION]\n"
                      "   -h, --help         show this page\n"
                      "   -i, --info         show informations\n"
                      "   -v, --version      show WMFS version\n");
               exit(EXIT_SUCCESS);
               break;
          case 'i':
               printf("WMFS - Window Manager From Scratch. By :\n"
                      "   - Martin Duquesnoy (code)\n"
                      "   - Marc Lagrange (build system)\n");
               exit(EXIT_SUCCESS);
               break;
          case 'v':
               printf("WMFS version : "WMFS_VERSION".\n"
                      "  Compilation settings :\n"
                      "    - Flags : "WMFS_COMPILE_FLAGS"\n"
                      "    - Linked Libs : "WMFS_LINKED_LIBS"\n"
                      "    - On "WMFS_COMPILE_MACHINE" by "WMFS_COMPILE_BY".\n");
               exit(EXIT_SUCCESS);
               break;
          }
     }

     if(!(dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "WMFS: cannot open X server.\n");
          exit(EXIT_FAILURE);
     }

     /* Set signal handle */
     sig.sa_handler = handle_signal;
     sig.sa_flags   = 0;
     memset(&sig.sa_mask, 0, sizeof(sigset_t));
     sigaction(SIGTERM, &sig, NULL);
     sigaction(SIGINT, &sig, NULL);

     /* Check if an other WM is already running; set the error handler */
     XSetErrorHandler(errorhandler);

     /* Let's Go ! */
     init_conf();
     init();
     scan();
     mainloop();
     raise(SIGTERM);

     XCloseDisplay(dpy);

     return 0;
}

