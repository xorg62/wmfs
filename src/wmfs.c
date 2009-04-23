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
     Client *c;

     /* Check if there are another WM running */
     if(BadAccess == event->error_code
        && ROOT == event->resourceid)
     {
          fprintf(stderr, "WMFS Error: Another Window Manager is already running.\n");
          exit(EXIT_FAILURE);
     }

     /* Ignore focus change error for unmapped client */
     /* Too lazy to add Xproto.h so:
      * 42 = X_SetInputFocus
      * 28 = X_GrabButton
      */
     if((c = client_gb_win(event->resourceid)))
          if(event->error_code == BadWindow
             || event->request_code == 42
               ||  event->request_code == 28)
               return 0;


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
     Client *c;
     int i;

     /* Set the silent error handler */
     XSetErrorHandler(errorhandlerdummy);

     /* Unmanage all clients */
     for(c = clients; c; c = c->next)
     {
          if(c->hide)
               client_unhide(c);
          if(c->unmapped)
               client_map(c);

          client_unmanage(c);
     }

     IFREE(tags);
     IFREE(seltag);

     XftFontClose(dpy, font);
     XFreeCursor(dpy, cursor[CurNormal]);
     XFreeCursor(dpy, cursor[CurMove]);
     XFreeCursor(dpy, cursor[CurResize]);
     infobar_destroy();

     IFREE(sgeo);
     IFREE(spgeo);
     IFREE(infobar);
     IFREE(keys);
     IFREE(func_list);
     IFREE(layout_list);

     /* Clean conf alloced thing {{{ */
     IFREE(menulayout.item);

     if(conf.menu)
     {
          for(i = 0; i < LEN(conf.menu); ++i)
               IFREE(conf.menu[i].item);
          IFREE(conf.menu);
     }

     IFREE(conf.launcher);
     IFREE(conf.ntag);
     IFREE(conf.titlebar.mouse);
     for(i = 0; i < conf.titlebar.nbutton; ++i)
     {
          IFREE(conf.titlebar.button[i].mouse);
          IFREE(conf.titlebar.button[i].linecoord);
     }

     IFREE(conf.bars.mouse);
     IFREE(conf.titlebar.button);
     IFREE(conf.client.mouse);
     IFREE(conf.root.mouse);
     /* }}} */

     XSync(dpy, False);

     return;
}

/** WMFS main loop: Check stdin and
 *  execute the event loop
 */
void
mainloop(void)
{
     XEvent ev;
     fd_set fd;

     while(!exiting)
     {
          if(QLength(dpy) > 0)
               XNextEvent(dpy, &ev);
          else
          {
               XFlush(dpy);
               FD_ZERO(&fd);
               FD_SET(ConnectionNumber(dpy), &fd);
               ev.type = LASTEvent;
               if(select(FD_SETSIZE, &fd, NULL, NULL, NULL) > 0)
                    XNextEvent(dpy, &ev);
          }
          getevent(ev);
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

/** Scan if there are windows on X
 *  for manage it
*/
void
scan(void)
{
     uint i, n;
     XWindowAttributes wa;
     Window usl, usl2, *w = NULL;
     Atom rt;
     int rf, tag = -1, screen = -1;
     ulong ir, il;
     uchar *ret;
     Client *c;

     if(XQueryTree(dpy, ROOT, &usl, &usl2, &w, &n))
          for(i = 0; i < n; ++i)
               if(XGetWindowAttributes(dpy, w[i], &wa)
                  && !(wa.override_redirect || XGetTransientForHint(dpy, w[i], &usl))
                  && wa.map_state == IsViewable)
               {
                    if(XGetWindowProperty(dpy, w[i], ATOM("_WMFS_TAG"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
                    {
                         tag = *ret;
                         XFree(ret);
                    }

                    if(XGetWindowProperty(dpy, w[i], ATOM("_WMFS_SCREEN"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
                    {
                         screen = *ret;
                         XFree(ret);
                    }

                    c = client_manage(w[i], &wa, False);

                    if(tag != -1)
                         c->tag = tag;
                    if(screen != -1 && screen <= screen_count() - 1)
                         c->screen = screen;

                    client_update_attributes(c);
               }

     for(i = 0; i < screen_count(); ++i)
          arrange(i);

     XFree(w);

     return;
}

/** Reload the WMFS with the new configuration
 * file changement *TESTING*
*/
void
uicb_reload(uicb_t cmd)
{
     quit();
     XCloseDisplay(dpy);

     for(; argv_global[0] && argv_global[0] == ' '; ++argv_global);

     execlp(argv_global, argv_global, NULL);

     return;
}

/** Check if wmfs is running (for function that will be
    execute when wmfs will be already running).
    \return False if wmfs is not running
*/
Bool
check_wmfs_running(void)
{
      Atom rt;
      int rf;
      ulong ir, il;
      uchar *ret;

      XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_RUNNING"), 0L, 4096,
                         False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret);

      if(!ret)
      {
           XFree(ret);

           fprintf(stderr, "Wmfs is not running. ( _WMFS_RUNNING not present)\n");

           return False;
      }

      XFree(ret);

      return True;
}

/** Execute an uicb function
 *\param func Function name
 *\param cmd Function's command
 *\return 0 if there is an error
*/
void
exec_uicb_function(char *func, char *cmd)
{
     XEvent ev;
     int i;
     long data[5];

     /* Check if wmfs is running (this function is executed when wmfs
      is already running normally...) */
     if(!check_wmfs_running())
          return;

     data[4] = True;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_FUNCTION"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (uchar*)func, strlen(func));

     if(cmd == NULL)
          cmd = "";

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_CMD"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (uchar*)cmd, strlen(cmd));


     ev.xclient.type = ClientMessage;
     ev.xclient.serial = 0;
     ev.xclient.send_event = True;
     ev.xclient.message_type = ATOM("_WMFS_FUNCTION");
     ev.xclient.window = ROOT;
     ev.xclient.format = 32;

     for(i = 0; i < 6; ++i)
          ev.xclient.data.l[i] = data[i];

     XSendEvent(dpy, ROOT, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
     XSync(dpy, False);

     return;
}

/** Set statustext
 *\param str Statustext string
*/
void
set_statustext(char *str)
{
     XEvent ev;
     int i;
     long data[5];

     data[4] = True;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_STATUSTEXT"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (unsigned char*)str, strlen(str));

     ev.xclient.type = ClientMessage;
     ev.xclient.serial = 0;
     ev.xclient.send_event = True;
     ev.xclient.message_type = ATOM("_WMFS_STATUSTEXT");
     ev.xclient.window = ROOT;
     ev.xclient.format = 32;

     for(i = 0; i < 6; ++i)
          ev.xclient.data.l[i] = data[i];

     XSendEvent(dpy, ROOT, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
     XSync(dpy, False);

     return;
}

/** Signal handle function
 * \param signum Signal number
*/
void
handle_signal(int signum)
{
     if(signum == SIGTERM || signum == SIGINT)
     {
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

     argv_global = _strdup(argv[0]);

     while ((i = getopt(argc, argv, "hvic:s:")) != -1)
     {
          if(i == 'c' || i == 's')
               if(!(dpy = XOpenDisplay(NULL)))
               {
                    fprintf(stderr, "WMFS: cannot open X server.\n");
                    exit(EXIT_FAILURE);
               }

          switch (i)
          {
          case 'h':
          default:
               printf("usage: %s [-ihv] [-c <uicb function> <cmd> ] [-s <string>]\n"
                      "   -c <uicb_function> <cmd>  Execute an uicb function to control WMFS\n"
                      "   -s <string>               Set the bar(s) statustext\n"
                      "   -h                        Show this page\n"
                      "   -i                        Show informations\n"
                      "   -v                        Show WMFS version\n", argv[0]);
               exit(EXIT_SUCCESS);
               break;
          case 'i':
               printf("WMFS - Window Manager From Scratch By Martin Duquesnoy \n");
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
          case 'c':
               exec_uicb_function(argv[2], ((argv[3]) ? argv[3] : NULL));
               XCloseDisplay(dpy);
               exit(EXIT_SUCCESS);
               break;
          case 's':
               set_statustext(optarg);
               XCloseDisplay(dpy);
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
     init();
     scan();
     mainloop();
     quit();

     XCloseDisplay(dpy);

     return 0;
}

