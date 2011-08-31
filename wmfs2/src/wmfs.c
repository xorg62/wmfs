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
#include "fifo.h"

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

static void
wmfs_xinit(void)
{
     char **misschar, **names, *defstring;
     int d, i, j;
     XModifierKeymap *mm;
     XFontStruct **xfs = NULL;
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
      * Font
      */
     setlocale(LC_CTYPE, "");

     W->font.fontset = XCreateFontSet(W->dpy, "fixed", &misschar, &d, &defstring);

     XExtentsOfFontSet(W->font.fontset);
     XFontsOfFontSet(W->font.fontset, &xfs, &names);

     W->font.as    = xfs[0]->max_bounds.ascent;
     W->font.de    = xfs[0]->max_bounds.descent;
     W->font.width = xfs[0]->max_bounds.width;

     W->font.height = W->font.as + W->font.de;

     if(misschar)
          XFreeStringList(misschar);

     /*
      * Keys
      */
     mm = XGetModifierMapping(W->dpy);

     for(i = 0; i < 8; i++)
          for(j = 0; j < mm->max_keypermod; ++j)
               if(mm->modifiermap[i * mm->max_keypermod + j]
                         == XKeysymToKeycode(W->dpy, XK_Num_Lock))
                    W->numlockmask = (1 << i);
     XFreeModifiermap(mm);

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
     Keybind *k;

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
     {
          while(W->running && !XNextEvent(W->dpy, &ev))
          {
               HANDLE_EVENT(&ev);
               fifo_read();
          }
     }
}

static inline void
wmfs_init(void)
{
     wmfs_xinit();
     ewmh_init();
     screen_init();
     event_init();
     config_init();
     fifo_init();
}

void
wmfs_quit(void)
{
     /* X stuffs */
     XFreeFontSet(W->dpy, W->font.fontset);

     screen_free();

     XCloseDisplay(W->dpy);

     free(W->net_atom);
     free(W);

     /* Conf stuffs */
     FREE_LIST(Keybind, W->h.keybind);

     /* FIFO stuffs */
     fclose(W->fifo.fp);
     free(W->fifo.path);

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
     W = (struct Wmfs*)xcalloc(1, sizeof(struct Wmfs));


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
