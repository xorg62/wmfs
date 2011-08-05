/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "event.h"

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
     int d, i = 0, j = 0;
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
     W->gc = DefaultGC(W->dpy, W->xscreen);

     /*
      * Root window
      */
     W->root = RootWindow(dpy, W->xscreen);
     at.event_mask = KeyMask | ButtonMask | MouseMask | PropertyChangeMask
          | SubstructureRedirectMask | SubstructureNotifyMask | StructureNotifyMask;
     XChangeWindowAttributes(W->dpy, W->root, CWEventMask | CWCursor, &at);

     /*
      * Font
      */
     setlocale(LC_CTYPE, "");

     W->font.fontset = XCreateFontSet(W->dpy, "fixed", &misschar, &d, &defstring);

     XExtentsOfFontSet(W->font.fontset);
     XFontsOfFontSet(W->font.fontset, &xfs, &names);

     font.as    = xfs[0]->max_bounds.ascent;
     font.de    = xfs[0]->max_bounds.descent;
     font.width = xfs[0]->max_bounds.width;

     W->font.height = W->font.as + W->font.de;

     if(misschar)
          XFreeStringList(misschar);

     /*
      * Keys
      */
     mm = XGetModifierMapping(W->dpy);

     for(; i < 8; i++)
          for(; j < mm->max_keypermod; ++j)
               if(mm->modifiermap[i * mm->max_keypermod + j]
                         == XKeysymToKeycode(W->dpy, XK_Num_Lock))
                    W->numlockmask = (1 << i);
     XFreeModifiermap(mm);

}

static void
wmfs_loop(void)
{
     XEvent ev;

     W->running = true;

     while(W->running)
          while(XPending(W->dpy))
          {
               XNextEvent(W->dpy, &ev);
               HANDLE_EVENT(&e);
          }
}

static void
wmfs_init(void)
{
     /* X init */
     wmfs_xinit();

     /* Screen init */
     screen_init();

}

void
wmfs_quit(void)
{
     W->running = false;

     /* X stuffs */
     XCloseDisplay(W->dpy);
     XFreeGC(W->gc);
     XFreeFontSet(dpy, W->font.fontset);

     screen_free();

     free(W);
}


int
main(int argc, char **argv)
{
     W = xcalloc(1, sizeof(Wmfs));

     /* Get X display */
     if(!(W->dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "%s: Can't open X server\n", argv[0]);
          exit(EXIT_FAILURE);
     }

     /* Opt */

     wmfs_init();

     wmfs_loop();

     wmfs_quit();

     return 1;
}
