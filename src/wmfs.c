/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef HAVE_IMLIB2
#include <Imlib2.h>
#endif /* HAVE_IMLIB2 */

#include "wmfs.h"
#include "event.h"
#include "ewmh.h"
#include "screen.h"
#include "infobar.h"
#include "util.h"
#include "config.h"
#include "client.h"
#include "layout.h"
#include "systray.h"

int
wmfs_error_handler(Display *d, XErrorEvent *event)
{
      char mess[256];

      /* Check if there is another WM running */
      if(event->error_code == BadAccess
                && W->root == event->resourceid)
           errl(EXIT_FAILURE, "Another Window Manager is already running.");

      /* Ignore focus change error for unmapped client
       * 42 = X_SetInputFocus
       * 28 = X_GrabButton
       */
     if(client_gb_win(event->resourceid))
          if(event->error_code == BadWindow
                    || event->request_code == 42
                    || event->request_code == 28)
               return 0;


     if(XGetErrorText(d, event->error_code, mess, 128))
          warnxl("%s(%d) opcodes %d/%d\n  resource #%lx\n",
                    mess,
                    event->error_code,
                    event->request_code,
                    event->minor_code,
                    event->resourceid);

     return 1;
}

int
wmfs_error_handler_dummy(Display *d, XErrorEvent *event)
{
     (void)d;
     (void)event;

     return 0;
}

void
wmfs_numlockmask(void)
{
     int i, j;
     XModifierKeymap *mm = XGetModifierMapping(W->dpy);

     for(i = 0; i < 8; i++)
          for(j = 0; j < mm->max_keypermod; ++j)
               if(mm->modifiermap[i * mm->max_keypermod + j]
                         == XKeysymToKeycode(W->dpy, XK_Num_Lock))
                    W->numlockmask = (1 << i);

     XFreeModifiermap(mm);
}

#ifdef HAVE_XFT
void
wmfs_init_font(char *font, struct theme *t)
{
     if(!(t->font = XftFontOpenName(W->dpy, W->xscreen, font)))
     {
          warnxl("Can't load font '%s'", font);
          t->font = XftFontOpenName(W->dpy, W->xscreen, "fixed");
     }
}
#else
void
wmfs_init_font(char *font, struct theme *t)
{
     XFontStruct **xfs = NULL;
     char **misschar, **names, *defstring;
     int d;

     if(!(t->font.fontset = XCreateFontSet(W->dpy, font, &misschar, &d, &defstring)))
     {
          warnxl("Can't load font '%s'", font);
          t->font.fontset = XCreateFontSet(W->dpy, "fixed", &misschar, &d, &defstring);
     }

     XExtentsOfFontSet(t->font.fontset);
     XFontsOfFontSet(t->font.fontset, &xfs, &names);

     t->font.as    = xfs[0]->max_bounds.ascent;
     t->font.de    = xfs[0]->max_bounds.descent;
     t->font.width = xfs[0]->max_bounds.width;

     t->font.height = t->font.as + t->font.de;

     if(misschar)
          XFreeStringList(misschar);
}
#endif /* HAVE_XFT */

static void
wmfs_xinit(void)
{
     XGCValues xgc =
     {
          .function       = GXinvert,
          .subwindow_mode = IncludeInferiors,
          .line_width     = 1
     };

     XSetWindowAttributes at =
     {
          .event_mask = (KeyMask | ButtonMask | MouseMask
                    | PropertyChangeMask | SubstructureRedirectMask
                    | SubstructureNotifyMask | StructureNotifyMask),
          .cursor = XCreateFontCursor(W->dpy, XC_left_ptr)
     };

     /*
      * X Error handler
      */
     XSetErrorHandler(wmfs_error_handler);

     /*
      * X var
      */
     W->xscreen = DefaultScreen(W->dpy);
     W->xdepth = DefaultDepth(W->dpy, W->xscreen);
     W->gc = DefaultGC(W->dpy, W->xscreen);
     W->xmaxw = DisplayWidth(W->dpy, W->xscreen);
     W->xmaxh = DisplayHeight(W->dpy, W->xscreen);

     /*
      * Keys
      */
     wmfs_numlockmask();

     /*
      * Root window/cursor
      */
     W->root = RootWindow(W->dpy, W->xscreen);
     XChangeWindowAttributes(W->dpy, W->root, CWEventMask | CWCursor, &at);
     W->rgc = XCreateGC(W->dpy, W->root, GCFunction | GCSubwindowMode | GCLineWidth, &xgc);


     /*
      * Locale (font encode)
      */
     setlocale(LC_CTYPE, "");

     /*
      * Barwin linked list
      */
     SLIST_INIT(&W->h.barwin);
     SLIST_INIT(&W->h.vbarwin);

     /*
      * Optional dep init
      */
#ifdef HAVE_IMLIB2
     imlib_context_set_display(W->dpy);
     imlib_context_set_visual(DefaultVisual(W->dpy, W->xscreen));
     imlib_context_set_colormap(DefaultColormap(W->dpy, W->xscreen));
#endif /* HAVE_IMLIB2 */

     W->flags |= WMFS_RUNNING;
}

void
wmfs_grab_keys(void)
{
     KeyCode c;
     struct keybind *k;

     wmfs_numlockmask();

     XUngrabKey(W->dpy, AnyKey, AnyModifier, W->root);

     SLIST_FOREACH(k, &W->h.keybind, next)
          if((c = XKeysymToKeycode(W->dpy, k->keysym)))
          {
               XGrabKey(W->dpy, c, k->mod, W->root, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(W->dpy, c, k->mod | LockMask, W->root, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(W->dpy, c, k->mod | W->numlockmask, W->root, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(W->dpy, c, k->mod | LockMask | W->numlockmask, W->root, True, GrabModeAsync, GrabModeAsync);
          }
}

/** Scan xprops of previous session to set it back
  * Check if there are windows on X (previous sessions windows)
*/
static void
wmfs_scan(void)
{
     struct geo g;
     struct client *c, *cc, *fc;
     struct screen *s;
     int i, n, rf, nscreen = 0;
     int tag = -1, screen = -1, flags = -1;
     unsigned long ir, il;
     long *ret = NULL, *tret = NULL;
     bool getg = false;
     XWindowAttributes wa;
     Window usl, usl2, *w = NULL, tm, focus;
     Atom rt;

     SLIST_INIT(&W->h.client);

     W->flags |= WMFS_SCAN;

     /* Get previous selected tag to apply it at the end */
     if(XGetWindowProperty(W->dpy, W->root, W->net_atom[wmfs_current_tag], 0, 32,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il,
                           (unsigned char**)&tret)
               == Success && tret)
     {
          nscreen = (int)ir;
     }

     /* Previous focused client before reload */
     if(XGetWindowProperty(W->dpy, W->root, W->net_atom[wmfs_focus], 0, 32,
                           False, XA_WINDOW, &rt, &rf, &ir, &il,
                           (unsigned char**)&ret)
               == Success && ret)
     {
          focus = *ret;
          XFree(ret);
     }


     if(XQueryTree(W->dpy, W->root, &usl, &usl2, &w, (unsigned int*)&n))
          for(i = n - 1; i != -1; --i)
          {
               XGetWindowAttributes(W->dpy, w[i], &wa);

               if(!wa.override_redirect && wa.map_state == IsViewable)
               {
                    if(ewmh_get_xembed_state(w[i]))
                    {
                         systray_add(w[i]);
                         continue;
                    }

                    if(ewmh_manage_window_type_desktop(w[i]))
                         continue;

                    if(XGetWindowProperty(W->dpy, w[i], ATOM("_WMFS_TAG"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il,
                                          (unsigned char**)&ret)
                              == Success && ret)
                    {
                         tag = *ret;
                         XFree(ret);
                    }

                    if(XGetWindowProperty(W->dpy, w[i], ATOM("_WMFS_SCREEN"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il,
                                          (unsigned char**)&ret)
                              == Success && ret)
                    {
                         screen = *ret;
                         XFree(ret);
                    }

                    if(XGetWindowProperty(W->dpy, w[i], ATOM("_WMFS_FLAGS"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il,
                                          (unsigned char**)&ret)
                              == Success && ret)
                    {
                         flags = *ret;
                         flags &= ~(CLIENT_TABBED | CLIENT_REMOVEALL);
                         XFree(ret);
                    }

                    if(XGetWindowProperty(W->dpy, w[i], ATOM("_WMFS_GEO"), 0, 32,
                                          False, XA_CARDINAL, &rt, &rf, &ir, &il,
                                          (unsigned char**)&ret)
                              == Success && ret)
                    {
                         g.x = ret[0];
                         g.y = ret[1];
                         g.w = ret[2];
                         g.h = ret[3];

                         getg = true;
                         XFree(ret);
                    }

                    if(XGetWindowProperty(W->dpy, w[i], ATOM("_WMFS_TABMASTER"), 0, 32,
                                          False, XA_WINDOW, &rt, &rf, &ir, &il,
                                          (unsigned char**)&ret)
                              == Success && ret)
                    {
                         tm = *ret;
                         XFree(ret);
                    }

                    c = client_new(w[i], &wa, true);

                    if(tm != c->win)
                         c->tmp = tm;
                    tm = 0;

                    if(flags != -1)
                         c->flags |= flags;

                    if(tag != -1 && screen != -1)
                    {
                         c->screen = screen_gb_id(screen);

                         if(getg)
                              c->flags |= CLIENT_IGNORE_LAYOUT;

                         tag_client(tag_gb_id(c->screen, tag), c);

                         if(getg && tag <= TAILQ_LAST(&c->screen->tags, tsub)->id - 1)
                              client_moveresize(c, &g);
                         /* In a removed tag */
                         else
                         {
                              c->geo = g;
                              layout_client(c);
                         }

                         client_get_name(c);
                    }
               }
          }

     if(!nscreen)
     {
          SLIST_FOREACH(s, &W->h.screen, next)
               if(!TAILQ_EMPTY(&s->tags))
                    tag_screen(s, TAILQ_FIRST(&s->tags));
     }
     else
     {
          /* Set back selected tag */
          for(i = 0; i < nscreen; ++i)
          {
               s = screen_gb_id(i);
               tag_screen(s, tag_gb_id(s, tret[i]));
          }
     }

     /* Re-adjust tabbed clients */
     SLIST_FOREACH(c, &W->h.client, next)
          if((cc = client_gb_win(c->tmp)) && cc != c)
               _client_tab(c, cc);

     if((fc = client_gb_win(focus)) && fc != W->client)
          client_focus(fc);

     SLIST_FOREACH(c, &W->h.client, next)
          if(c->flags & CLIENT_TILED)
               layout_fix_hole(c);

     W->flags &= ~WMFS_SCAN;

     if(tret)
          XFree(tret);

     XFree(w);
     XSync(W->dpy, false);
}

static inline void
wmfs_sigchld(void)
{
     if(W->flags & WMFS_SIGCHLD)
     {
          while(waitpid(-1, NULL, WNOHANG) > 0);
          W->flags ^= WMFS_SIGCHLD;
     }
}

static void
wmfs_loop(void)
{
     XEvent ev;

     while((W->flags & WMFS_RUNNING) && !XNextEvent(W->dpy, &ev))
     {
          /* Manage SIGCHLD event here, X is not safe with it */
          wmfs_sigchld();
          EVENT_HANDLE(&ev);
     }
}

static inline void
wmfs_init(void)
{
     log_init();
     wmfs_xinit();
     ewmh_init();
     screen_init();
     event_init();
     config_init();
}

void
wmfs_quit(void)
{
     struct keybind *k;
     struct rule *r;
     struct theme *t;
     struct client *c;
     struct mousebind *m;
     struct launcher *l;

     ewmh_update_wmfs_props();

     XFreeGC(W->dpy, W->rgc);

     while(!SLIST_EMPTY(&W->h.client))
     {
          c = SLIST_FIRST(&W->h.client);
          client_update_props(c, CPROP_LOC | CPROP_FLAG | CPROP_GEO);
          c->flags |= (CLIENT_IGNORE_LAYOUT | CLIENT_REMOVEALL);
          XMapWindow(W->dpy, c->win);
          client_remove(c);
     }

     /* Will free:
      *
      * Screens -> tags
      *         -> Infobars -> Elements
      */
     screen_free();

     /* Conf stuffs */
     while(!SLIST_EMPTY(&W->h.theme))
     {
          t = SLIST_FIRST(&W->h.theme);
          SLIST_REMOVE_HEAD(&W->h.theme, next);
#ifdef HAVE_XFT
          XftFontClose(W->dpy, t->font);
#else
          XFreeFontSet(W->dpy, t->font.fontset);
#endif /* HAVE_XFT */
          status_free_ctx(&t->tags_n_sl);
          status_free_ctx(&t->tags_s_sl);
          status_free_ctx(&t->tags_o_sl);
          status_free_ctx(&t->tags_u_sl);
          status_free_ctx(&t->client_n_sl);
          status_free_ctx(&t->client_s_sl);
          free(t);
     }

     while(!SLIST_EMPTY(&W->h.keybind))
     {
          k = SLIST_FIRST(&W->h.keybind);
          SLIST_REMOVE_HEAD(&W->h.keybind, next);
          free((void*)k->cmd);
          free(k);
     }

     while(!SLIST_EMPTY(&W->h.mousebind))
     {
          m = SLIST_FIRST(&W->h.mousebind);
          SLIST_REMOVE_HEAD(&W->h.mousebind, globnext);
          free((void*)m->cmd);
          free(m);
     }

     while(!SLIST_EMPTY(&W->h.launcher))
     {
          l = SLIST_FIRST(&W->h.launcher);
          SLIST_REMOVE_HEAD(&W->h.launcher, next);
          free((void*)l->name);
          free((void*)l->prompt);
          free((void*)l->command);
          free(l);
     }

     while(!SLIST_EMPTY(&W->h.rule))
     {
          r = SLIST_FIRST(&W->h.rule);
          SLIST_REMOVE_HEAD(&W->h.rule, next);
          free(r->class);
          free(r->instance);
          free(r->role);
          free(r->name);
          free(r);
     }

     /* close log */
     if(W->log)
          fclose(W->log), W->log = NULL;

     W->flags &= ~WMFS_RUNNING;
}

/** Reload WMFS binary
*/
void
uicb_reload(Uicb cmd)
{
     (void)cmd;

     W->flags &= ~WMFS_RUNNING;
     W->flags |= WMFS_RELOAD;
}

void
uicb_quit(Uicb cmd)
{
     (void)cmd;

     W->flags &= ~WMFS_RUNNING;
}

static void
exec_uicb_function(Display *dpy, Window root, char *func, char *cmd)
{
     Atom utf8s = XInternAtom(dpy, "UTF8_STRING", false);
     XClientMessageEvent e = {
          .type         = ClientMessage,
          .message_type = XInternAtom(dpy, "_WMFS_FUNCTION", false),
          .window       = root,
          .format       = 32,
          .data.l[4]    = true
     };

     XChangeProperty(dpy,root, XInternAtom(dpy, "_WMFS_FUNCTION", false), utf8s,
                     8, PropModeReplace, (unsigned char*)func, strlen(func));

     if(!cmd)
          cmd = "";

     XChangeProperty(dpy, root, XInternAtom(dpy, "_WMFS_CMD", false), utf8s,
                     8, PropModeReplace, (unsigned char*)cmd, strlen(cmd));

     XSendEvent(dpy, root, false, StructureNotifyMask, (XEvent*)&e);
     XSync(dpy, False);
}

static void
signal_handle(int sig)
{
     switch (sig)
     {
     case SIGQUIT:
     case SIGTERM:
          W->flags &= ~WMFS_RUNNING;
          break;
     case SIGCHLD:
          W->flags |= WMFS_SIGCHLD;
          break;
     }
}

int
main(int argc, char **argv)
{
     int i;
     bool r;
     Display *dpy;
     char path[MAX_PATH_LEN] = { 0 };
     struct sigaction sa;
     (void)argc;

     sprintf(path, "%s/"CONFIG_DEFAULT_PATH, getenv("HOME"));

     /* Opt */
     while((i = getopt(argc, argv, "hvC:c:")) != -1)
     {
          switch(i)
          {
               default:
               case 'h':
                    printf("usage: %s [-hv] [-c <func> <cmd] [-C <file>]\n"
                           "   -h                Show this page\n"
                           "   -v                Show WMFS version\n"
                           "   -c <func> <cmd>   Execute a specified UICB function\n"
                           "   -C <file>         Launch WMFS with a specified configuration file\n", argv[0]);
                    exit(EXIT_SUCCESS);
                    break;

               case 'v':
                    printf("wmfs("WMFS_VERSION") 2 beta\n");
                    exit(EXIT_SUCCESS);
                    break;

               case 'c':
                    if(!(dpy = XOpenDisplay(NULL)))
                    {
                         fprintf(stderr, "%s: Can't open X server\n", argv[0]);
                         exit(EXIT_FAILURE);
                    }

                    exec_uicb_function(dpy,  DefaultRootWindow(dpy), optarg, argv[optind]);

                    XCloseDisplay(dpy);
                    exit(EXIT_SUCCESS);
               break;

               case 'C':
                    strncpy(path, optarg, MAX_PATH_LEN);
                    break;
          }
     }

     W = (struct wmfs*)xcalloc(1, sizeof(struct wmfs));

     /* Default path ~/.config/wmfs/wmfsrc */
     W->confpath = path;

     /* Get X display */
     if(!(W->dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "%s: Can't open X server\n", argv[0]);
          exit(EXIT_FAILURE);
     }

     /* Set signal handler */
     memset(&sa, 0, sizeof(sa));
     sa.sa_handler = signal_handle;
     sigemptyset(&sa.sa_mask);
     sigaction(SIGQUIT, &sa, NULL);
     sigaction(SIGTERM, &sa, NULL);
     sigaction(SIGCHLD, &sa, NULL);

     /* Core */
     wmfs_init();
     wmfs_scan();
     wmfs_loop();
     wmfs_quit();

     r = (W->flags & WMFS_RELOAD);
     free(W);

     if(r)
          execvp(argv[0], argv);

     XCloseDisplay(W->dpy);

     return 1;
}
