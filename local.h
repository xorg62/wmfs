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
#include <sys/time.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include "config.h"

/* DEFINE TYPES */
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)
#define CONTROL      ControlMask
#define ALT          Mod1Mask
#define SHIFT        ShiftMask
#define ITOA(p,n)    sprintf(p,"%i",n)
#define debug(p)     printf("debug: %i\n", p)
#define Move         0
#define Resize       1
#define MAXTAG       36
#define NBUTTON      5

typedef struct Client Client;
struct Client {
     char *title;          /* Client title */
     int tag;              /* Tag num */
     int x, y, w, h;       /* Window attribute */
     int ox, oy, ow, oh;   /* Old window attribute */
     /* For resizehint usage { */
     int basew, baseh, incw, inch;
     int maxw, maxh, minw, minh;
     int minax, maxax, minay, maxay;
     /* } */
     int border;           /* border height */
     Window win;           /* Window */
     Window tbar;          /* Titlebar */
     Window button;        /* Close Button */
     Bool max, tile, free; /* Client Info */
     Bool hint, hide;      /* Client InfoÂ² */
     Client *next;         /* Next client */
     Client *prev;         /* Previous client */
};

typedef struct {
     unsigned long mod;
     KeySym keysym;
     void (*func)(char *cmd);
     char *cmd;
} Key;

typedef struct {
     char *text;
     Window win;
     int fg_color;
     int bg_color;
     unsigned int x;
     int nmousesec;
     void (*func[NBUTTON])(char *cmd);
     char *cmd[NBUTTON];
     unsigned int mouse[NBUTTON];
} BarButton;

typedef struct {
     char *name;
     float mwfact;
     int nmaster;
     int layout;
} Tag;

typedef struct {
     char *font;
     char *buttonfont;
     bool raisefocus;
     bool raiseswitch;
     int borderheight;
     int ttbarheight;
     struct {
          int bordernormal;
          int borderfocus;
          int bar;
          int text;
          int tagselfg;
          int tagselbg;
     } colors;
     struct {
          char *free;
          char *tile;
          char *max;
     } layouts;
     Tag tag[MAXTAG];
     BarButton barbutton[64];
     int ntag;
     int nkeybind;
     int nbutton;
} Conf;

enum { CurNormal, CurResize, CurMove, CurInput, CurLast };
enum { WMState, WMProtocols, WMName, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };
enum { Free=0, Tile, Max};

/* wmfs.c */
void arrange(void);
void attach(Client *c);
void buttonpress(XEvent *event);
int clienthintpertag(int tag);
int clientpertag(int tag);
void configurerequest(XEvent event);
void detach(Client *c);
void *emallocz(unsigned int size);
int errorhandler(Display *d, XErrorEvent *event);
int errorhandlerdummy(Display *d, XErrorEvent *event);
void focus(Client *c);
void freelayout(void);
Client* getbutton(Window w);
Client* getclient(Window w);
Client* getnext(Client *c);
char* getlayoutsym(int l);
Client* gettbar(Window w);
void getevent(void);
void grabbuttons(Client *c, Bool focused);
void grabkeys(void);
void hide(Client *c);
void init(void);
Bool ishide(Client *c);
void keymovex(char *cmd);
void keymovey(char *cmd);
void keypress(XEvent *e);
void keyresize(char *cmd);
void killclient(char *cmd);
void layoutswitch(char *cmd);
void lowerclient(Client *c);
void mapclient(Client *c);
void manage(Window w, XWindowAttributes *wa);
void maxlayout(void);
void mouseaction(Client *c, int x, int y, int type);
void moveresize(Client *c, int x, int y, int w, int h, bool r);
Client *nexttiled(Client *c);
void raiseclient(Client *c);
void scan(void);
void setborder(Window win, int color);
void set_mwfact(char *cmd);
void set_nmaster(char *cmd);
void setsizehints(Client *c);
void spawn(char *cmd);
void tag(char *cmd);
void tagtransfert(char *cmd);
void tile(void);
void tile_switch(char *cmd);
void togglemax(char *cmd);
void unhide(Client *c);
void unmanage(Client *c);
void updatebar(void);
void updatebutton(Bool c);
void unmapclient(Client *c);
void updateall(void);
void updatetitle(Client *c);
void wswitch(char *cmd);

#define BUTY(y)      (y - conf.ttbarheight + 3)
#define BUTH         (conf.ttbarheight - 6)

Display *dpy;
GC gc;
XEvent event;
XFontStruct *font, *font_b;
Conf conf;
int screen;
Window root;
Window bar;
fd_set fd;
struct tm *tm;
time_t lt;
Key keys[512];
Atom wm_atom[WMLast];
Atom net_atom[NetLast];
Cursor cursor[CurLast];
int mw, mh;
int fonth;
int fonty;
int barheight;
int seltag;
Client *clients;                     /* First Client */
Client *sel;                         /* selected client */
Client *selbytag[MAXTAG];

/* layout */
float mwfact[MAXTAG];
int nmaster[MAXTAG];
int layout[MAXTAG];
void (*layoutfunc[MAXTAG])(void);
/**/

char bartext[1024];

#endif /* LOCAL_H */
