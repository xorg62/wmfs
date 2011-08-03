/*
*      wmfs.c
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

#include "wmfs.h"

static volatile bool exiting = False, sig_chld = False;

int
errorhandler(Display *d, XErrorEvent *event)
{
     char mess[256];

     /* Check if there is another WM running */
     if(BadAccess == event->error_code
        && ROOT == event->resourceid)
          errx(EXIT_FAILURE, "Another Window Manager is already running.");

     /* Ignore focus change error for unmapped client */
     /* Too lazy to add Xproto.h so:
      * 42 = X_SetInputFocus
      * 28 = X_GrabButton
      */
     if(client_gb_win(event->resourceid))
          if(event->error_code == BadWindow
             || event->request_code == 42
               ||  event->request_code == 28)
               return 0;


     XGetErrorText(d, event->error_code, mess, 128);
     warnx("%s(%d) opcodes %d/%d\n  resource #%lx\n", mess,
             event->error_code,
             event->request_code,
             event->minor_code,
             event->resourceid);

     return 1;
}

int
errorhandlerdummy(Display *d, XErrorEvent *event)
{
     (void)d;
     (void)event;
     return 0;
}

/** Clean wmfs before the exit
 */
void
quit(void)
{
     size_t i, len;

     /* Set the silent error handler */
     XSetErrorHandler(errorhandlerdummy);

     /* Unmanage all clients */
     while(!SLIST_EMPTY(&clients))
     {
          Client *c = SLIST_FIRST(&clients);
          client_unhide(c);
          XReparentWindow(dpy, c->win, ROOT, c->geo.x, c->geo.y);
          free(c);
          SLIST_REMOVE_HEAD(&clients, next);
     }

     free(tags);
     free(seltag);

     systray_freeicons();

#ifdef HAVE_XFT
     if(conf.use_xft)
          XftFontClose(dpy, font.font);
     else
#endif /* HAVE_XFT */
          XFreeFontSet(dpy, font.fontset);

     for(i = 0; i < CurLast; ++i)
          XFreeCursor(dpy, cursor[i]);
     XFreeGC(dpy, gc_stipple);
     infobar_destroy();

     /* BarWindows */
     while(!SLIST_EMPTY(&bwhead))
          SLIST_REMOVE_HEAD(&bwhead, next);

     free(sgeo);
     free(spgeo);
     free(infobar);
     free(keys);
     free(net_atom);

     /* Clean conf alloced thing */
     free(menulayout.item);

     if(conf.menu)
     {
          len = LEN(conf.menu);
          for(i = 0; i < len; ++i)
               free(conf.menu[i].item);
          free(conf.menu);
     }

     free(conf.launcher);
     free(conf.rule);

     free(conf.bars.mouse);
     free(conf.selbar.mouse);
     free(conf.titlebar.button);
     free(conf.client.mouse);
     free(conf.root.mouse);

     free_conf();

     XSync(dpy, False);
     XCloseDisplay(dpy);

     free(event_handle);

     return;
}

/** WMFS main loop.
 */
static void
mainloop(void)
{
     XEvent ev;

     while(!exiting && !XNextEvent(dpy, &ev))
          HANDLE_EVENT(&ev);

     return;
}

/** Set the exiting variable to True
 *  for stop the main loop
 * \param cmd unused uicb_t
 */
void
uicb_quit(uicb_t cmd)
{
     (void)cmd;
     exiting = True;

     return;
}

/** Scan if there are windows on X
 *  for manage it
*/
static void
scan(void)
{
     uint n;
     XWindowAttributes wa;
     Window usl, usl2, *w = NULL;
     Atom rt;
     int s, rf, tag = -1, screen = -1, flags = -1, i;
     ulong ir, il;
     uchar *ret;
     Client *c;

     s = screen_count();

     if(XQueryTree(dpy, ROOT, &usl, &usl2, &w, &n))
          for(i = n - 1; i != -1; --i)
          {
               XGetWindowAttributes(dpy, w[i], &wa);

               if(!wa.override_redirect && wa.map_state == IsViewable)
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

                    if(XGetWindowProperty(dpy, w[i], ATOM("_WMFS_FLAGS"), 0, 32,
                                   False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
                    {
                         flags = *ret;
                         XFree(ret);
                    }

                    c = client_manage(w[i], &wa, False);

                    if(tag != -1 && tag != MAXTAG + 1)
                         c->tag = tag;
                    if(screen != -1 && screen <= s - 1)
                         c->screen = screen;
                    if(flags != -1)
                         c->flags = flags;

                    client_update_attributes(c);
               }
          }

     /* Set update layout request */
     SLIST_FOREACH(c, &clients, next)
     {
          if(c->tag > conf.ntag[c->screen])
               c->tag = conf.ntag[c->screen];
          tags[c->screen][c->tag].flags |= RequestUpdateFlag;
     }

     for(i = 0; i < s; ++i)
          arrange(i, True);

     XFree(w);

     return;
}

/** Reload the WMFS with the new configuration
 * file changement *TESTING*
*/
void
uicb_reload(uicb_t cmd)
{
     (void)cmd;
     quit();

     for(; argv_global[0] && argv_global[0] == ' '; ++argv_global);

     /* add -C to always load the same config file */
     execvp(argv_global, all_argv);

     return;
}

/** Check if wmfs is running (for functions that will be
    execute when wmfs will be already running).
    \return False if wmfs is not running
*/
bool
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

           warnx("Wmfs is not running. (_WMFS_RUNNING not present)");

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
     /* Check if wmfs is running (this function is executed when wmfs
      is already running normally...) */
     if(!check_wmfs_running())
          return;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_FUNCTION"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (uchar*)func, strlen(func));

     if(cmd == NULL)
          cmd = "";

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_CMD"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (uchar*)cmd, strlen(cmd));

     ewmh_send_message(ROOT, ROOT, "_WMFS_FUNCTION", 0, 0, 0, 0, True);

     return;
}

/** Set statustext
 *\param str Statustext string
*/
static void
set_statustext(int s, char *str)
{
     int i;
     char atom_name[64];

     if(!str)
          return;

     if(s == -1)
     {
          for(i = 0; i < screen_count(); ++i)
          {
               sprintf(atom_name, "_WMFS_STATUSTEXT_%d", i);

               XChangeProperty(dpy, ROOT, ATOM(atom_name), ATOM("UTF8_STRING"),
                         8, PropModeReplace, (uchar*)str, strlen(str));

               ewmh_send_message(ROOT, ROOT, atom_name, 0, 0, 0, 0, True);
          }
     }
     else
     {
          sprintf(atom_name, "_WMFS_STATUSTEXT_%d", s);

          XChangeProperty(dpy, ROOT, ATOM(atom_name), ATOM("UTF8_STRING"),
                         8, PropModeReplace, (uchar*)str, strlen(str));

               ewmh_send_message(ROOT, ROOT, atom_name, 0, 0, 0, 0, True);
     }

     return;
}

/** Signal handle function
*/
static void
signal_handle(int sig)
{
     switch (sig)
     {
          case SIGQUIT:
          case SIGTERM:
               exiting = True;
               break;
          case SIGCHLD:
               sig_chld = True;
               break;
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
     char *ol = "cs";
     extern char *optarg;
     extern int optind;
     struct sigaction sa;

     argv_global  = xstrdup(argv[0]);
     all_argv = argv;
     sprintf(conf.confpath, "%s/"DEF_CONF, getenv("HOME"));

     while((i = getopt(argc, argv, "hvic:s:C:")) != -1)
     {

          /* For options who need WMFS running */
          if(strchr(ol, i) && !(dpy = XOpenDisplay(NULL)))
               errx(EXIT_FAILURE, "cannot open X server.");

          switch(i)
          {
          case 'h':
          default:
               printf("usage: %s [-ihv] [-C <file>] [-c <uicb function> <cmd> ] [-g <argument>] [-s <screen_num> <string>]\n"
                      "   -C <file>                 Load a configuration file\n"
                      "   -c <uicb_function> <cmd>  Execute an uicb function to control WMFS\n"
                      "   -s <screen_num> <string>  Set the bar(s) statustext\n"
                      "   -h                        Show this page\n"
                      "   -i                        Show informations\n"
                      "   -v                        Show WMFS version\n", argv[0]);
               exit(EXIT_SUCCESS);
               break;

          case 'i':
               printf("WMFS - Window Manager From Scratch By Martin Duquesnoy\n");
               exit(EXIT_SUCCESS);
               break;

          case 'v':
               printf("wmfs"WMFS_VERSION"\n");
               exit(EXIT_SUCCESS);
               break;

          case 'C':
               strncpy(conf.confpath, optarg, sizeof(conf.confpath));
               break;

          case 'c':
               exec_uicb_function(optarg, argv[optind]);
               XCloseDisplay(dpy);
               exit(EXIT_SUCCESS);
               break;

          case 's':
               if(argc > 3)
                    set_statustext(atoi(optarg), argv[3]);
               else
                    set_statustext(-1, optarg);
               XCloseDisplay(dpy);
               exit(EXIT_SUCCESS);
               break;
          }
     }

     /* Check if WMFS can open X server */
     if(!(dpy = XOpenDisplay(NULL)))
          errx(EXIT_FAILURE, "cannot open X server.");

     /* Set signal handler */
     memset(&sa, 0, sizeof(sa));
     sa.sa_handler = signal_handle;
     sigemptyset(&sa.sa_mask);
     sigaction(SIGQUIT, &sa, NULL);
     sigaction(SIGTERM, &sa, NULL);
     sigaction(SIGCHLD, &sa, NULL);

     /* Check if an other WM is already running; set the error handler */
     XSetErrorHandler(errorhandler);

     /* Let's Go ! */
     init();
     scan();
     mainloop();
     quit();

     return 0;
}

