#ifndef LOCAL_H
#define LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <confuse.h>
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
#define FALSE        0
#define TRUE         (!FALSE)
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)
#define CONTROL      ControlMask
#define ALT          Mod1Mask
#define SHIFT        ShiftMask
#define LEN(x)       (sizeof x / sizeof x[0])
#define Move         0
#define Resize       1
#define Free         0
#define Tile         1
#define Max          2
#define MAXTAG       36

typedef struct Client Client;
struct Client {
     char *title;          /* client title */
     int tag;              /* tag num */
     int x, y, w, h;       /* window attribute */
     int ox, oy, ow, oh;   /* old window attribute */
     int border;           /* border height */
     Window win;           /* window */
     Window tbar;          /* Titlebar? */
     Window button;        /* Close Button */
     Bool max;             /* client info */
     int layout;
     Client *next;         /* next client */
     Client *prev;         /* previous client */
};

typedef struct {
     unsigned long mod;
     KeySym keysym;
     void (*func)(char *cmd);
     char *cmd;
} Key;

typedef struct {

     char *name;
     void *func;

} func_name_list_t;

typedef struct {
     /* bool and size */
     char *font;
     bool raisefocus;
     bool raiseswitch;
     int borderheight;
     int ttbarheight;
     /* color */
     int bordernormal;
     int borderfocus;
     int barcolor;
     int buttoncolor;
     int textcolor;
     int tagselfg;
     int tagselbg;
     /* layout */
     char *symlayout[3];
     /* tag */
     int ntag;
     char *taglist[MAXTAG];
} Conf;

enum { CurNormal, CurResize, CurMove, CurInput, CurLast };
enum { WMState, WMProtocols, WMName, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* wmfs.c */
void attach(Client *c);
int clientpertag(int tag);
void detach(Client *c);
void *emallocz(unsigned int size);
int errorhandler(Display *d, XErrorEvent *event);
void focus(Client *c);
Client* getbutton(Window w);
Client* getclient(Window w);
Client* getnext(Client *c);
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
void mapclient(Client *c);
void manage(Window w, XWindowAttributes *wa);
void mouseaction(Client *c, int x, int y, int type);
void moveresize(Client *c, int x, int y, int w, int h);
void raiseclient(Client *c);
void scan(void);
void setborder(Window win, int color);
void spawn(char *cmd);
void tag(char *cmd);
void tagn(int tag);
void tagswitch(char *cmd);
void tile(char *cmd);
void togglemax(char *cmd);
void unhide(Client *c);
void unmanage(Client *c);
void updatebar(void);
void unmapclient(Client *c);
void updateall(void);
void updatetitle(Client *c);
void wswitch(char *cmd);

#define BUTY(y)      (y - conf.ttbarheight + 3)
#define BUTH         (conf.ttbarheight - 6)

GC gc;
Key keys[1024];
XEvent event;
Display *dpy;
XFontStruct* font;
Conf conf;
int screen;
Window root;
Window bar;
fd_set fd;
Atom wm_atom[WMLast];
Atom net_atom[NetLast];
Cursor cursor[CurLast];
int mw, mh;
int fonth;
int barheight;
Client *clients;                     /* Fisrt Client */
Client *sel;                         /* selected client */
int seltag;                          /* selected tag */
#endif /* LOCAL_H */
