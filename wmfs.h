/*
*      wmfs.h
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

#ifndef LOCAL_H
#define LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/xpm.h>
#include <confuse.h>
#include "config.h"


/* Defines */
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)
#define ALT          Mod1Mask
#define ITOA(p ,n)   sprintf(p, "%i", n)
#define debug(p)     fprintf(stderr, "debug: %i\n", p)
#define MAXTAG       36
#define NBUTTON      5
#define BUTY(y)      y - conf.ttbarheight + 3
#define BUTH         conf.ttbarheight - 6
#define BUTX(x, w)   x + w - BUTH/400
#define MAXLAYOUT    3
#define PAD          8
#define BPAD         2

/* Client Structure  & Typedef */
typedef const char* uicb_t;
typedef struct Client Client;

/* Bar Window Structure:
 * For titlebar or topbar.. */
typedef struct
{
     Window win;
     Drawable dr;
     int x, y;
     uint w ,h;
     uint color;
     int bord;
} BarWindow;

struct Client
{
     char *title;          /* Client title */
     int tag;              /* Tag num */
     int x, y, w, h;       /* Window attribute */
     int ox, oy, ow, oh;   /* Old window attribute */
     /* For resizehint usage { */
     int basew, baseh, incw, inch;
     int maxw, maxh, minw, minh;
     int minax, maxax, minay, maxay;
     /* } */
     Window win;           /* Window */
     BarWindow *tbar;      /* Titlebar */
     Window button;        /* Close Button */
     Bool max, tile, free; /* Client Info */
     Bool hint, hide, lmax;/* Client Info² */
     /* Struct in chains */
     Client *next;         /* Next client */
     Client *prev;         /* Previous client */
};

/* Keybind Structure */
typedef struct
{
     uint mod;
     KeySym keysym;
     void (*func)(uicb_t);
     char *cmd;
} Key;

/* Bar Button */
typedef struct
{
     char *text;
     BarWindow *bw;
     char *fg_color;
     int bg_color;
     uint x;
     int nmousesec;
     void (*func[NBUTTON])(uicb_t);
     char *cmd[NBUTTON];
     uint mouse[NBUTTON];
} BarButton;

/* Layout Structure */
typedef struct
{
     char *image;
     void (*func)(void);
} Layout;

/* Tag Structure */
typedef struct
{
     char *name;
     float mwfact;
     int nmaster;
     Bool resizehint;
     Layout layout;
} Tag;

/* Configuration structure */
typedef struct
{
     char *font;
     bool raisefocus;
     bool raiseswitch;
     bool bartop;
     int borderheight;
     int ttbarheight;
     int tagbordwidth;
     struct
     {
          /* Only the colors will be use for text
           * are 'char*' (for xprint -> XftColorAllocName) */
          uint bordernormal;
          uint borderfocus;
          uint bar;
          char *text;
          char *tagselfg;
          uint tagselbg;
          uint tagbord;
          char *layout_fg;
          uint layout_bg;
          char *ttbar_text_focus;
          char *ttbar_text_normal;
          uint button;
          uint button_border;
     } colors;
     Tag tag[MAXTAG];
     Layout layout[MAXLAYOUT];
     BarButton *barbutton;
     int ntag;
     int nkeybind;
     int nbutton;
     int nlayout;
} Conf;

/* Config.c struct */
typedef struct
{
     char *name;
     void *func;
} func_name_list_t;

typedef struct
{
      char *name;
      KeySym keysym;
} key_name_list_t;

typedef struct
{
     char *name;
     uint button;
} name_to_uint_t;

typedef struct
{
     char *name;
     char *content;
} Variable;

/* Enum */
enum { CurNormal, CurResize, CurMove, CurLast };
enum { WMState, WMProtocols, WMName, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* Functions Prototypes */

/* bar.c */
BarWindow* bar_create(int x, int y, uint w, uint h, int bord, uint color, Bool entermask);
void bar_delete(BarWindow *bw);
void bar_moveresize(BarWindow *bw, int x, int y, uint w, uint h);
void bar_refresh_color(BarWindow *bw);
void bar_refresh(BarWindow *bw);
void updatebar(void);
void updatebutton(Bool c);
void updatetitlebar(Client *c);
void uicb_togglebarpos(uicb_t);

/* draw.c */
void draw_text(Drawable d, int x, int y, char* fg, uint bg, int pad, char *str);
void draw_image(Drawable dr, int x, int y, char *file);
void draw_taglist(Drawable dr);
void draw_layout(int x, int y);
void draw_rectangle(Drawable dr, int x, int y, uint w, uint h, uint color);
XImage* get_image_attribute(char *file);
ushort textw(const char *text);

/* config.c */
void init_conf(void);

/* event.c */
void buttonpress(XEvent ev);
void configurerequest(XEvent ev);
void destroynotify(XEvent ev);
void enternotify(XEvent ev);
void expose(XEvent ev);
void focusin(XEvent ev);
void keypress(XEvent ev);
void mapnotify(XEvent ev);
void maprequest(XEvent ev);
void propertynotify(XEvent ev);
void unmapnotify(XEvent ev);
void getevent(void);

/* util.c */
void *emalloc(uint elemet, uint size);
ulong getcolor(char *color);
void uicb_spawn(uicb_t);

/* tag.c */
void uicb_tag(uicb_t);
void uicb_tag_next(uicb_t);
void uicb_tag_prev(uicb_t);
void uicb_tagtransfert(uicb_t);

/* layout.c */
void freelayout(void);
void layoutswitch(Bool b);
void maxlayout(void);
Client* nexttiled(Client *c);
void tile(void);
void uicb_tile_switch(uicb_t);
void uicb_togglemax(uicb_t);
void uicb_layout_prev(uicb_t);
void uicb_layout_next(uicb_t);
void uicb_set_mwfact(uicb_t);
void uicb_set_nmaster(uicb_t);

/* wmfs.c */
void arrange(void);
void attach(Client *c);
int clientpertag(int tag);
void client_switch(Bool c);
void detach(Client *c);
int errorhandler(Display *d, XErrorEvent *event);
int errorhandlerdummy(Display *d, XErrorEvent *event);
void focus(Client *c);
Client* getbutton(Window w);
Client* getclient(Window w);
Client* getnext(Client *c);
Client* gettbar(Window w);
void grabbuttons(Client *c, Bool focused);
void grabkeys(void);
void hide(Client *c);
void init(void);
Bool ishide(Client *c);
void mainloop(void);
void mapclient(Client *c);
void manage(Window w, XWindowAttributes *wa);
void mouseaction(Client *c, int x, int y, int type);
void moveresize(Client *c, int x, int y, int w, int h, bool r);
void raiseclient(Client *c);
void scan(void);
void setborder(Window win, int color);
void setwinstate(Window win, long state);
void setsizehints(Client *c);
void unhide(Client *c);
void unmanage(Client *c);
void unmapclient(Client *c);
void uicb_client_prev(uicb_t);
void uicb_client_next(uicb_t);
void uicb_killclient(uicb_t);
void uicb_quit(uicb_t);

/* Variables */

/* Principal */
Display *dpy;
XEvent event;
GC gc;
Window root;
int screen;
int mw, mh;
Conf conf;
Key *keys;
Bool exiting;

/* Atoms / Cursors */
Atom wm_atom[WMLast];
Atom net_atom[NetLast];
Cursor cursor[CurLast];

/* Fonts */
int fonth;
XftFont *xftfont;

/* Bar / Tags */
BarWindow *bar;
BarWindow *layoutsym;
Tag tags[MAXTAG];
int barheight;
char bartext[1024];
int seltag;
int taglen[MAXTAG];
int bary;

/* Important Client */
Client *clients;
Client *sel;
Client *selbytag[MAXTAG];

/* Other */
uint numlockmask;
Variable confvar[256];

#endif /* LOCAL_H */


