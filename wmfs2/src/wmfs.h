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
typedef enum { BarTop = 0, BarBottom, BarHide, BarLast } Barpos;
typedef enum { Right = 0, Left, Top, Bottom, Center, PositionLast } Position;

/*
 * Structures
 */

struct Geo
{
     int x, y, w, h;
};

/* struct Barwin */
struct Barwin
{
     struct Geo geo;
     Window win;
     Drawable dr;
     Color fg, bg;
      Flags flags;
     void *ptr; /* Special cases */
     SLIST_HEAD(, Mousebind) mousebinds;
     SLIST_ENTRY(Barwin) next;  /* global barwin */
     SLIST_ENTRY(Barwin) enext; /* element barwin */
};

/* struct Infobar's element */
struct Element
{
     struct Geo geo;
     struct Infobar *infobar;
     int type;
     void (*func_init)(struct Element *e);
     void (*func_update)(struct Element *e);
     SLIST_HEAD(, Barwin) bars;
     TAILQ_ENTRY(Element) next;
};

/* struct Infobar */
struct Infobar
{
     struct Barwin *bar;
     struct Geo geo;
     struct Scr33n *screen;
     struct Theme *theme;
     char *elemorder;
     Barpos pos;
     TAILQ_HEAD(esub, Element) elements;
     SLIST_ENTRY(Infobar) next;
};

/* Screen */
struct Scr33n
{
     struct Geo geo, ugeo;
     struct Tag *seltag;
     int id;
     Flags elemupdate;
     TAILQ_HEAD(tsub, Tag) tags;
     SLIST_HEAD(, Infobar) infobars;
     SLIST_ENTRY(Scr33n) next;
};

/* struct Tag */
struct Tag
{
     struct Scr33n *screen;
     struct Client *sel;
     struct Frame *frame;
     char *name;
     Flags flags;
     SLIST_HEAD(, Frame) frames;
     SLIST_HEAD(, Client) clients;
     TAILQ_ENTRY(Tag) next;
};

/* struct Client */
struct Client
{
     struct Tag *tag;
     struct Scr33n *screen;
     struct Frame *frame;
     struct Barwin *titlebar;
     struct Geo geo;
     char *title;
     Flags flags;
     Window win;
     SLIST_ENTRY(Client) next;  /* Global list */
     SLIST_ENTRY(Client) tnext; /* struct Tag list */
     SLIST_ENTRY(Client) fnext; /* struct struct Frame list */
};

/* struct struct Frame */
struct Frame
{
     struct Tag *tag;
     struct Geo geo;
     Window win;
     Color fg, bg;
     SLIST_HEAD(, Client) clients;
     SLIST_ENTRY(Frame) next;
};

/* Config */
struct Keybind
{
     unsigned int mod;
     void (*func)(Uicb);
     Uicb cmd;
     KeySym keysym;
     SLIST_ENTRY(Keybind) next;
};

struct Mousebind
{
     struct Geo area;
     unsigned int button;
     bool use_area;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(Mousebind) next;
};

struct Colpair
{
     Color fg, bg;
};

struct Theme
{
     char *name;

     /* Font */
     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;

     /* Bars */
     struct Colpair bars;
     int bars_width;

     /* struct Elements */
     struct Colpair tags_n, tags_s; /* normal / selected */
     int tags_border_width;
     Color tags_border_col;

     /* struct Client / struct struct Frame */
     struct Colpair client_n, client_s;
     Color frame_bg;
     int client_titlebar_width;
     int client_border_width;

     SLIST_ENTRY(Theme) next;
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
     bool running;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, Scr33n) screen;
          SLIST_HEAD(, Client) client;
          SLIST_HEAD(, Keybind) keybind;
          SLIST_HEAD(, Barwin) barwin;
          SLIST_HEAD(, Theme) theme;
     } h;

     /*
      * Selected screen, client
      */
     struct Scr33n *screen;
     struct Client *client;

};

int wmfs_error_handler(Display *d, XErrorEvent *event);
int wmfs_error_handler_dummy(Display *d, XErrorEvent *event);
void wmfs_grab_keys(void);
void wmfs_numlockmask(void);
void wmfs_init_font(char *font, struct Theme *t);
void wmfs_quit(void);
void uicb_reload(Uicb cmd);
void uicb_quit(Uicb cmd);


/* Single global variable */
struct Wmfs *W;

#endif /* WMFS_H */
