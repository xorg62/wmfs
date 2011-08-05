/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef WMFS_H
#define WMFS_H

/* Standard */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <err.h>
#include <sys/queue.h>

/* Xlib */
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* Local */
#include "screen.h"
#include "client.h"

#define ButtonMask   (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)

typedef struct
{
     int x, y, w, h;
} Geo;

typedef struct
{
     /* X11 stuffs */
     bool running;
     Display *dpy;
     Window root;
     int xscreen;
     Flags numlockmask;
     GC gc;

     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, Screen) screen;
          SLIST_HEAD(, Client) client;
     } h;

     /*
      * Selected screen, from what you go everywhere; selected tag,
      * and then selected client.
      */
     Screen *screen;

} Wmfs;


int wmfs_error_handler(Display *d, XErrorEvent *event);
int wmfs_error_handler_dummy(Display *d, XErrorEvent *event);
void wmfs_quit(void);

/* Single global variable */
Wmfs *W;

#endif /* WMFS_H */
