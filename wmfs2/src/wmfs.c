/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <getopt.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include "wmfs.h"
#include "event.h"
#include "ewmh.h"
#include "screen.h"
#include "infobar.h"
#include "util.h"
#include "config.h"
#include "client.h"

int
wmfs_error_handler(Display *d, XErrorEvent *event)
{
      char mess[256];

      /* Check if there is another WM running */
      if(event->error_code == BadAccess
                && W->root == event->resourceid)
           errx(EXIT_FAILURE, "Another Window Manager is already running.");

      /* Ignore focus change error for unmapped client
       * 42 = X_SetInputFocus
       * 28 = X_GrabButton
       */
     if(client_gb_win(event->resourceid))
          if(event->error_code == BadWindow
                    || event->request_code == 42
                    || event->request_code == 28)
               return 0;


     XGetErrorText(d, event->error_code, mess, 128);
     warnx("%s(%d) opcodes %d/%d\n  resource #%lx\n",
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

void
wmfs_init_font(char *font, struct theme *t)
{
     XFontStruct **xfs = NULL;
     char **misschar, **names, *defstring;
     int d;

     if(!(t->font.fontset = XCreateFontSet(W->dpy, font, &misschar, &d, &defstring)))
     {
          warnx("Can't load font '%s'", font);
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

static void
wmfs_xinit(void)
{
     int i, j;
     XModifierKeymap *mm;
     XSetWindowAttributes at;

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

     /*
      * Root window/cursor
      */
     W->root = RootWindow(W->dpy, W->xscreen);

     at.event_mask = KeyMask | ButtonMask | MouseMask | PropertyChangeMask
          | SubstructureRedirectMask | SubstructureNotifyMask | StructureNotifyMask;
     at.cursor = XCreateFontCursor(W->dpy, XC_left_ptr);

     XChangeWindowAttributes(W->dpy, W->root, CWEventMask | CWCursor, &at);

     /*
      * Locale (font encode)
      */
     setlocale(LC_CTYPE, "");

     /*
      * Keys
      */
     wmfs_numlockmask();

     /*
      * Barwin linked list
      */
     SLIST_INIT(&W->h.barwin);

     W->running = true;
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

/** Scan if there are windows on X
 *  for manage it
*/
static void
wmfs_scan(void)
{
     int i, n;
     XWindowAttributes wa;
     Window usl, usl2, *w = NULL;

     SLIST_INIT(&W->h.client);

     /*
        Atom rt;
        int s, rf, tag = -1, screen = -1, flags = -1, i;
        ulong ir, il;
        uchar *ret;
      */

     if(XQueryTree(W->dpy, W->root, &usl, &usl2, &w, (unsigned int*)&n))
          for(i = n - 1; i != -1; --i)
          {
               XGetWindowAttributes(W->dpy, w[i], &wa);

               if(!wa.override_redirect && wa.map_state == IsViewable)
               {/*
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
                 */
                    /*c = */ client_new(w[i], &wa);

                    /*
                    if(tag != -1)
                         c->tag = tag;
                    if(screen != -1)
                         c->screen = screen;
                    if(flags != -1)
                         c->flags = flags;
                    */
               }
          }

     XFree(w);
}

static void
wmfs_loop(void)
{
     XEvent ev;

     while(XPending(W->dpy))
          while(W->running && !XNextEvent(W->dpy, &ev))
               HANDLE_EVENT(&ev);
}

static inline void
wmfs_init(void)
{
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
     struct theme *t;

     /* Will free:
      *
      * Screens -> tags
      *         -> Infobars -> Elements
      */
     screen_free();

     XCloseDisplay(W->dpy);

     /* Conf stuffs */
     while(!SLIST_EMPTY(&W->h.keybind))
     {
          k = SLIST_FIRST(&W->h.keybind);
          SLIST_REMOVE_HEAD(&W->h.keybind, next);
          free((void*)k->cmd);
          free(k);
     }

     while(!SLIST_EMPTY(&W->h.theme))
     {
          t = SLIST_FIRST(&W->h.theme);
          SLIST_REMOVE_HEAD(&W->h.theme, next);
          XFreeFontSet(W->dpy, t->font.fontset);
          free(t);
     }

     free(W->net_atom);
     free(W);

     W->running = false;
}

/** Reload WMFS binary
*/
void
uicb_reload(Uicb cmd)
{
     (void)cmd;

}

void
uicb_quit(Uicb cmd)
{
     (void)cmd;
     W->running = false;
}

int
main(int argc, char **argv)
{
     W = (struct wmfs*)xcalloc(1, sizeof(struct wmfs));


     /* Get X display */
     if(!(W->dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "%s: Can't open X server\n", argv[0]);
          exit(EXIT_FAILURE);
     }

     /* Opt */
     /*
     int i;
     while((i = getopt(argc, argv, "hviC:")) != -1)
     {
          switch(i)
          {
               case 'h':
                    break;
               case 'v':
                    break;
               case 'C':
                    break;
          }
     }
     */

     /* Core */
     wmfs_init();
     wmfs_scan();

     wmfs_loop();

     wmfs_quit();

     return 1;
}
