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

#define ButtonMask (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask  (ButtonMask | PointerMotionMask)
#define KeyMask    (KeyPressMask | KeyReleaseMask)

typedef unsigned int Flags;
typedef unsigned int Color;
typedef const char* Uicb;

/*
 * Structures
 */
typedef struct Geo Geo;
typedef struct Barwin Barwin;
typedef struct Scr33n Scr33n;
typedef struct Tag Tag;
typedef struct Client Client;
typedef struct Keybind Keybind;
typedef struct Mousebind Mousebind;

struct Geo
{
     int x, y, w, h;
};

/* Barwin */
struct Barwin
{
     Window win;
     Drawable dr;
     Color fg, bg;
     Geo geo;
     Flags flags;
     SLIST_ENTRY(Barwin) next;
};

/* Infobar */
struct Infobar
{
     Barwin *bar;
};

/* Screen */
struct Scr33n
{
     Geo geo;
     Tag *seltag;
     SLIST_HEAD(, Tag) tags;
     SLIST_ENTRY(Scr33n) next;
};

/* Tag */
struct Tag
{
     char *name;
     Scr33n *screen;
     Flags flags;
     Client *sel;
     SLIST_ENTRY(Tag) next;
};

/* Client */
struct Client
{
     Tag *tag;
     Scr33n *screen;
     Geo geo;
     Flags flags;
     char *title;
     Window win;
     SLIST_ENTRY(Client) next;
};

/* Config */
struct Keybind
{
     unsigned int mod;
     KeySym keysym;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(Keybind) next;
};

struct Mousebind
{
     unsigned int button;
     Geo area;
     bool use_area;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(Keybind) next;
};

struct Config
{
     /* Misc section */
     struct
     {
          char *font;
          bool focus_follow_mouse;
          bool focus_follow_movement;
          bool focus_pointer_click;
     } misc;
};

/* Global struct */
struct Wmfs
{
     /* X11 stuffs */
     Display *dpy;
     Window root;
     int xscreen, xdepth;
     Flags numlockmask;
     GC gc;
     Atom *net_atom;

     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, Scr33n) screen;
          SLIST_HEAD(, Client) client;
          SLIST_HEAD(, Keybind) keybind;
          SLIST_HEAD(, Barwin) barwin;
     } h;

     /*
      * Selected screen, from what you go everywhere; selected tag,
      * and then selected client.
      */
     Scr33n *screen;

};

int wmfs_error_handler(Display *d, XErrorEvent *event);
int wmfs_error_handler_dummy(Display *d, XErrorEvent *event);
void wmfs_grab_keys(void);
void wmfs_quit(void);

/* Single global variable */
struct Wmfs *W;

#endif /* WMFS_H */
