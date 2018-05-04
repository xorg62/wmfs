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

#ifdef HAVE_XFT
/* Xft */
#include <X11/Xft/Xft.h>
#endif /* HAVE_XFT */

/* Local */
#include "log.h"

#define CONFIG_DEFAULT_PATH ".config/wmfs/wmfsrc"

#define ButtonMask (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask  (ButtonMask | PointerMotionMask)
#define KeyMask    (KeyPressMask | KeyReleaseMask)

typedef unsigned long Flags;
typedef unsigned int Color;
typedef const char* Uicb;

#ifdef HAVE_XFT
typedef Color BgColor;
typedef XftColor FgColor;
#else
typedef Color BgColor;
typedef Color FgColor;
#endif /* HAVE_XFT */

enum barpos
{
     BarTop = 0,
     BarBottom,
     BarHide,
     BarLast
};

enum position
{
     Right = 0,
     Left,
     Top,
     Bottom,
     Center,
     NoAlign,
     PositionLast
};

enum size_hints
{
     BASEW, BASEH,
     INCW,  INCH,
     MAXW,  MAXH,
     MINW,  MINH,
     MINAX, MINAY,
     MAXAX, MAXAY,
     SHLAST
};

/*
 * Structures
 */

struct geo
{
     int x, y, w, h;
};

struct geo_list
{
     struct geo geo;
     SLIST_ENTRY(geo_list) next;
};

struct colpair
{
     FgColor fg;
     BgColor bg;
};

struct barwin
{
     struct geo geo;
     Window win;
     Drawable dr;
#ifdef HAVE_XFT
     XftDraw *xftdraw;
#endif /* HAVE_XFT */
     FgColor fg;
     BgColor bg;
     void *ptr; /* Special cases */
     SLIST_HEAD(mbhead, mousebind) mousebinds;
     SLIST_HEAD(, mousebind) statusmousebinds;
     SLIST_ENTRY(barwin) next;  /* global barwin */
     SLIST_ENTRY(barwin) enext; /* element barwin */
     SLIST_ENTRY(barwin) vnext; /* volatile barwin */
};

struct status_seq
{
     struct geo geo;
     enum position align;
     int data[4];
     char type;
     char *str;
     FgColor fg;
     BgColor bg;
     BgColor bg2;
     SLIST_HEAD(, mousebind) mousebinds;
     SLIST_ENTRY(status_seq) next;
};

struct status_ctx
{
     struct barwin *barwin;
     struct theme *theme;
#define STATUS_BLOCK_REFRESH  0x01
#define STATUS_UPDATE         0x02
     Flags flags;
     char *status;
     SLIST_HEAD(, status_gcache) gcache;
     SLIST_HEAD(, status_seq) statushead;
};

struct status_gcache
{
     char *name;
     int *datas;
     int ndata;
     SLIST_ENTRY(status_gcache) next;
};

struct element
{
     struct geo geo;
     struct infobar *infobar;
     struct status_ctx *statusctx;
     int type;
     char *data;
     enum position align;
     void (*func_init)(struct element *e);
     void (*func_update)(struct element *e);
     SLIST_HEAD(, barwin) bars;
     TAILQ_ENTRY(element) next;
};

struct infobar
{
     struct barwin *bar;
     struct geo geo;
     struct screen *screen;
     struct theme *theme;
     struct status_ctx statusctx;
     enum barpos opos, pos;
     char *elemorder;
     char *name;
     TAILQ_HEAD(esub, element) elements;
     SLIST_ENTRY(infobar) next;
};

struct screen
{
     struct geo geo, ugeo;
     struct tag *seltag;
#define SCREEN_TAG_UPDATE 0x01
     Flags flags;
     int id;
     TAILQ_HEAD(tsub, tag) tags;
     SLIST_HEAD(, infobar) infobars;
     SLIST_ENTRY(screen) next;
};

SLIST_HEAD(chead, client);

struct tag
{
     struct screen *screen;
     struct client *sel;
     struct client *prevsel;
     struct tag *prev;
     struct status_ctx statusctx;
     char *name;
     int id;
#define TAG_URGENT       0x01
#define TAG_IGNORE_ENTER 0x02
     Flags flags;
     SLIST_HEAD(, client) clients;
     TAILQ_HEAD(ssub, layout_set) sets;
     TAILQ_ENTRY(tag) next;
};

struct client
{
     struct tag *tag, *prevtag;
     struct screen *screen;
     struct barwin *titlebar;
     struct geo geo, wgeo, tgeo, ttgeo, rgeo, *tbgeo;
     struct colpair ncol, scol;
     struct theme *theme;
     struct client *tabmaster;
     int sizeh[SHLAST];
     char *title;
     int border, tbarw;
#define CLIENT_HINT_FLAG     0x01
#define CLIENT_IGNORE_ENTER  0x02
#define CLIENT_DID_WINSIZE   0x04
#define CLIENT_FAC_APPLIED   0x08
#define CLIENT_IGNORE_LAYOUT 0x10
#define CLIENT_RULED         0x20
#define CLIENT_TABBED        0x40
#define CLIENT_TABMASTER     0x80
#define CLIENT_DYING         0x100 /* Saddest flag ever */
#define CLIENT_REMOVEALL     0x200
#define CLIENT_MAPPED        0x400
#define CLIENT_FULLSCREEN    0x800
#define CLIENT_FREE          0x1000
#define CLIENT_TILED         0x2000
#define CLIENT_MOUSE         0x4000
#define CLIENT_IGNORE_TAG    0x8000
     Flags flags;
     Window win, frame, tmp;
     SLIST_ENTRY(client) next;   /* Global list */
     SLIST_ENTRY(client) tnext;  /* struct tag list */
};

struct layout_set
{
     int n;
     SLIST_HEAD(, geo_list) geos;
     TAILQ_ENTRY(layout_set) next;
};

struct keybind
{
     unsigned int mod;
     void (*func)(Uicb);
     Uicb cmd;
     KeySym keysym;
     SLIST_ENTRY(keybind) next;
};

struct mousebind
{
     struct geo area;
     unsigned int button;
     bool use_area;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(mousebind) next;
     SLIST_ENTRY(mousebind) snext;
     SLIST_ENTRY(mousebind) globnext;
};

struct theme
{
     char *name;

     /* Font */
#ifdef HAVE_XFT
     XftFont *font;
#else
     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;
#endif /* HAVE_XFT */

     /* Bars */
     struct colpair bars;
     int bars_width;

     /* struct elements */
     struct colpair tags_n, tags_s, tags_o, tags_u; /* normal / selected / occupied */
     struct status_ctx tags_n_sl, tags_s_sl, tags_o_sl, tags_u_sl; /* status line */
     int tags_border_width;
     BgColor tags_border_col;

     /* client / frame */
     struct colpair client_n, client_s;
     struct status_ctx client_n_sl, client_s_sl, client_f_sl;
     BgColor frame_bg;
     int client_titlebar_width;
     int client_border_width;

     SLIST_ENTRY(theme) next;
};

struct rule
{
     struct theme *theme;
     char *class;
     char *instance;
     char *role;
     char *name;
     char *client_machine;
     int tag, screen;
#define RULE_FREE       0x01
#define RULE_TAB        0x02
#define RULE_IGNORE_TAG 0x04
     Flags flags;
     SLIST_ENTRY(rule) next;
};

struct launcher
{
     char *name;
     char *prompt;
     char *command;
#define HISTOLEN 64
     char histo[HISTOLEN][256];
     int nhisto;
     int width;
     SLIST_ENTRY(launcher) next;
};

struct launcher_ccache
{
     char *start;
     char **namelist;
     size_t hits;
};

struct _systray
{
     struct geo geo;
     Window win;
     SLIST_ENTRY(_systray) next;
};

#define MAX_PATH_LEN 8192

struct wmfs
{
     /* X11 stuffs */
     Display *dpy;
     Window root;
     int xscreen, xdepth;
     int xmaxw, xmaxh;
     int nscreen;
     unsigned int client_mod;
     Flags numlockmask;
#define WMFS_SCAN      0x001
#define WMFS_RUNNING   0x002
#define WMFS_RELOAD    0x004
#define WMFS_SYSTRAY   0x008
#define WMFS_LOG       0x010
#define WMFS_LAUNCHER  0x020
#define WMFS_SIGCHLD   0x040
#define WMFS_TABNOC    0x080 /* tab next opened client */
#define WMFS_TAGCIRC   0x100 /* tab_next on last tag -> go to first tag / tab_prev on first tag -> go to last tag */
#define WMFS_AUTOFOCUS 0x200
#define WMFS_IGN_ENTER 0x400
     Flags flags;
     GC gc, rgc;
     Atom *net_atom;
     char **argv;
     char *confpath;
     struct barwin *last_clicked_barwin;
     struct theme *ctheme;
#define CFOCUS_ENTER 0x01
#define CFOCUS_CLICK 0x02
     Flags cfocus; /* Focus configuration, can be set to 0, CFOCUS_ENTER or CFOCUS_CLICK*/

     int padding;

     /* Log file */
     FILE *log;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, screen) screen;
          SLIST_HEAD(, client) client;
          SLIST_HEAD(, keybind) keybind;
          SLIST_HEAD(, barwin) barwin;
          SLIST_HEAD(, theme) theme;
          SLIST_HEAD(, rule) rule;
          SLIST_HEAD(, mousebind) mousebind;
          SLIST_HEAD(, launcher) launcher;
          SLIST_HEAD(, barwin) vbarwin;
     } h;

     /*
      * Temporary head of mousebind list from config
      * Will be copied in barwin of clickable drawable
      * later in code
      */
     struct
     {
          struct mbhead tag;
          struct mbhead client;
          struct mbhead root;
     } tmp_head;

     /*
      * Because there is only one systray per display,
      * set struct there
      */
     struct
     {
          struct barwin *barwin;
          struct infobar *infobar;
          bool redim;
          Window win;
          SLIST_HEAD(, _systray) head;
     } systray;

     /*
      * Selected screen, client
      */
     struct screen *screen;
     struct client *client;

};

int wmfs_error_handler(Display *d, XErrorEvent *event);
int wmfs_error_handler_dummy(Display *d, XErrorEvent *event);
void wmfs_grab_keys(void);
void wmfs_numlockmask(void);
void wmfs_init_font(char *font, struct theme *t);
void wmfs_quit(void);
void uicb_reload(Uicb cmd);
void uicb_quit(Uicb cmd);

/* Single global variable */
struct wmfs *W;

#endif /* WMFS_H */
