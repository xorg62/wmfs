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

void
checkotherwm(void)
{
     owm = False;

     XSetErrorHandler(errorhandlerstart);
     XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
     XSync(dpy, False);

     if(owm)
     {
         fprintf(stderr, "WMFS Error: Another Window Manager is already running.\n");
         exit(EXIT_FAILURE);
     }

     XSetErrorHandler(errorhandler);
     XSync(dpy, False);

     return;
}

int
errorhandler(Display *d, XErrorEvent *event)
{
     char mess[512];

     XGetErrorText(d, event->error_code, mess, 128);
     fprintf(stderr, "WMFS error: %s(%d) opcodes %d/%d\n  resource 0x%lx\n", mess,
             event->error_code,
             event->request_code,
             event->minor_code,
             event->resourceid);

     return 1;
}

/* for no-important error */
int
errorhandlerdummy(Display *d, XErrorEvent *event)
{
     return 0;
}

/* Only for check if another WM is already running */
int
errorhandlerstart(Display *d, XErrorEvent *event)
{
     owm = True;

     return -1;
}

void
quit(void)
{
     int i;

     /* Exiting WMFS :'( */
     XftFontClose(dpy, xftfont);
     XFreeCursor(dpy, cursor[CurNormal]);
     XFreeCursor(dpy, cursor[CurMove]);
     XFreeCursor(dpy, cursor[CurResize]);
     bar_delete(bar);
     if(conf.nbutton)
          for(i = 0; i < conf.nbutton; ++i)
               bar_delete(conf.barbutton[i].bw);
     free(conf.barbutton);
     free(keys);
     free(clients);
     XSync(dpy, False);

     return;
}

void
init(void)
{
     XSetWindowAttributes at;
     XModifierKeymap *modmap;
     int i, j;

     /* FIRST INIT */
     gc = DefaultGC (dpy, screen);
     screen = DefaultScreen (dpy);
     root = RootWindow (dpy, screen);
     mw = DisplayWidth (dpy, screen);
     mh = DisplayHeight (dpy, screen);


     /* INIT TAG / LAYOUT ATTRIBUTE */
     seltag = 1;
     for(i = 0; i < conf.ntag + 1; ++i)
          tags[i] = conf.tag[i - 1];

     /* INIT FONT */
     xftfont = XftFontOpenName(dpy, screen, conf.font);
     if(!xftfont)
     {
          fprintf(stderr, "WMFS Error: Cannot initialize font\n");
          xftfont = XftFontOpenName(dpy, screen, "sans-10");
     }
     fonth = (xftfont->ascent + xftfont->descent) - 1;
     barheight = fonth + 4;


     /* INIT CURSOR */
     cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
     cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
     cursor[CurMove] = XCreateFontCursor(dpy, XC_fleur);

     /* INIT MODIFIER */
     modmap = XGetModifierMapping(dpy);
     for(i = 0; i < 8; i++)
          for(j = 0; j < modmap->max_keypermod; ++j)
               if(modmap->modifiermap[i * modmap->max_keypermod + j]
                  == XKeysymToKeycode(dpy, XK_Num_Lock))
                    numlockmask = (1 << i);
     XFreeModifiermap(modmap);

     /* INIT ATOM */
     wm_atom[WMState] = XInternAtom(dpy, "WM_STATE", False);
     wm_atom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
     wm_atom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
     net_atom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
     net_atom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
     XChangeProperty(dpy, root, net_atom[NetSupported], XA_ATOM, 32,
                     PropModeReplace, (unsigned char *) net_atom, NetLast);

     /* INIT ROOT */
     at.event_mask = KeyMask | ButtonPressMask | ButtonReleaseMask |
          SubstructureRedirectMask | SubstructureNotifyMask |
          EnterWindowMask | LeaveWindowMask | StructureNotifyMask ;
     at.cursor = cursor[CurNormal];
     XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &at);

     /* INIT BAR / BUTTON */
     bary = (conf.bartop) ? 0 : mh - barheight;
     bar = bar_create(0, bary, mw, barheight, 0, conf.colors.bar, False);
     XMapRaised(dpy, bar->win);
     strcpy(bartext, "WMFS-" WMFS_VERSION);
     updatebutton(False);
     updatebar();

     /* INIT WORKABLE SPACE */
     sgeo.x = 0;
     sgeo.y = (conf.bartop) ? barheight+1 : 1;
     sgeo.width = DisplayWidth(dpy, screen);
     sgeo.height = DisplayHeight(dpy, screen) - barheight;


     /* INIT STUFF */
     grabkeys();

     return;
}

void
mainloop(void)
{
     fd_set fd;
     char sbuf[sizeof bartext], *p;
     int len, r, offset = 0;
     Bool readstdin = True;

     len = sizeof bartext - 1;
     sbuf[len] = bartext[len] = '\0';

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
                              strncpy(bartext, sbuf, len);
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
                    strncpy(bartext, sbuf, strlen(sbuf));
                    readstdin = False;
               }
               updatebar();
          }
          while(XPending(dpy))
          {
               XNextEvent(dpy, &event);
               getevent();
          }
     }

     return;
}

void
uicb_quit(uicb_t cmd)
{
     exiting = True;

     return;
}

/* scan all the client who was in X before wmfs */
void
scan(void)
{
     uint i, num;
     Window *wins, d1, d2;
     XWindowAttributes wa;

     wins = NULL;
     if(XQueryTree(dpy, root, &d1, &d2, &wins, &num))
     {
          for(i = 0; i < num; i++)
          {
               if(!XGetWindowAttributes(dpy, wins[i], &wa))
                    continue;
               if(wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                    continue;
               if(wa.map_state == IsViewable)
                    client_manage(wins[i], &wa);
          }
     }
     if(wins)
          XFree(wins);

     arrange();

     return;
}

int
main(int argc, char **argv)
{
     int i;

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

     /* Check if an other WM is already running */
     checkotherwm();

     /* Let's Go ! */
     init_conf();
     init();
     scan();
     mainloop();
     quit();

     XCloseDisplay(dpy);

     return 0;
}

