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

#ifndef WMFS_H
#define WMFS_H

/* Lib headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <confuse.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>

/* Local headers */
#include "config.h"
#include "structs.h"

/* MACRO */
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)

#define MAXH         DisplayHeight(dpy, screen)
#define MAXW         DisplayWidth(dpy, screen)

#define FRAMEW(w)    w + conf.client.borderheight * 2
#define FRAMEH(h)    h + (conf.client.borderheight * 2) + conf.titlebar.height
#define BORDH        conf.client.borderheight
#define TBARH        conf.titlebar.height
#define RESHW        15

#define CHECK(x)     if(!x) return
#define ITOA(p ,n)   sprintf(p, "%d", n)
#define debug(p)     fprintf(stderr, "debug: %d\n", p)
#define PAD          8

/* bar.c */
BarWindow *bar_create(Window parent, int x, int y, uint w, uint h, int bord, uint color, Bool entermask);
void bar_delete(BarWindow *bw);
void bar_map(BarWindow *bw);
void bar_unmap(BarWindow *bw);
void bar_move(BarWindow *bw, int x, int y);
void bar_resize(BarWindow *bw, uint w, uint h);
void bar_refresh_color(BarWindow *bw);
void bar_refresh(BarWindow *bw);

/* draw.c */
void draw_text(Drawable d, int x, int y, char* fg, uint bg, int pad, char *str);
void draw_rectangle(Drawable dr, int x, int y, uint w, uint h, uint color);
ushort textw(const char *text);

/* infobar.c */
void infobar_init(void);
void infobar_draw(void);
void infobar_draw_layout(void);
void infobar_draw_taglist(void);
void uicb_infobar_togglepos(uicb_t cmd);

/* client.c */
int client_pertag(int tag);
void client_attach(Client *c);
void client_configure(Client *c);
void client_detach(Client *c);
void client_focus(Client *c);
/* client_gb_*() {{{ */
Client* client_gb_win(Window w);
Client* client_gb_frame(Window w);
Client* client_gb_titlebar(Window w);
Client* client_gb_resize(Window w);
/* }}} */
void client_get_name(Client *c);
void client_hide(Client *c);
Bool ishide(Client *c);
void client_map(Client *c);
void client_manage(Window w, XWindowAttributes *wa);
void client_moveresize(Client *c, XRectangle geo, bool r);
void client_size_hints(Client *c);
void client_raise(Client *c);
void client_unhide(Client *c);
void client_unmanage(Client *c);
void client_unmap(Client *c);
void uicb_client_raise(uicb_t cmd);
void uicb_client_prev(uicb_t);
void uicb_client_next(uicb_t);
void uicb_client_kill(uicb_t);

/* frame.c */
void frame_create(Client *c);
void frame_moveresize(Client *c, XRectangle geo);
void frame_update(Client *c);

/* config.c */
void init_conf(void);

/* event.c */
void buttonpress(XButtonEvent *ev);
void configurerequest(XConfigureRequestEvent *ev);
void destroynotify(XDestroyWindowEvent *ev);
void enternotify(XCrossingEvent *ev);
void expose(XExposeEvent *ev);
void focusin(XFocusChangeEvent *ev);
void grabkeys(void);
void keypress(XKeyPressedEvent *ev);
void mapnotify(XMappingEvent *ev);
void maprequest(XMapRequestEvent *ev);
void propertynotify(XPropertyEvent *ev);
void unmapnotify(XUnmapEvent *ev);
void getevent(XEvent ev);

/* mouse.c */
void mouse_move(Client *c);
void mouse_resize(Client *c);
void mouse_grabbuttons(Client *c, Bool focused);
void uicb_mouse_move(uicb_t cmd);
void uicb_mouse_resize(uicb_t cmd);

/* util.c */
void *emalloc(uint element, uint size);
void efree(void *ptr);
ulong getcolor(char *color);
long getwinstate(Window win);
double round(double x);
void setwinstate(Window win, long state);
void uicb_spawn(uicb_t);

/* tag.c */
void uicb_tag(uicb_t);
void uicb_tag_next(uicb_t);
void uicb_tag_prev(uicb_t);
void uicb_tagtransfert(uicb_t);

/* layout.c */
void arrange(void);
void freelayout(void);
void layoutswitch(Bool b);
void layout_tile_switch(Bool b);
void maxlayout(void);
Client *nexttiled(Client *c);
/* tile {{{ */
 void grid(void);
 void tile(void);
 void tile_left(void);
 void tile_top(void);
 void tile_bottom(void);
/* }}} */
void uicb_tile_switch(uicb_t);
void uicb_togglemax(uicb_t);
void uicb_togglefree(uicb_t cmd);
void uicb_layout_prev(uicb_t);
void uicb_layout_next(uicb_t);
void uicb_set_mwfact(uicb_t);
void uicb_set_nmaster(uicb_t);

/* init.c */
void init(void);
void init_atom(void);
void init_font(void);
void init_cursor(void);
void init_root(void);
void init_key(void);
void init_geometry(void);

/* wmfs.c */
void checkotherwm(void);
int errorhandler(Display *d, XErrorEvent *event);
int errorhandlerdummy(Display *d, XErrorEvent *event);
int errorhandlerstart(Display *d, XErrorEvent *event);
void quit(void);
void mainloop(void);
void scan(void);
void handle_signal(int signum);
void uicb_quit(uicb_t);

/* Variables */

/* Principal */
Display *dpy;
GC gc;
Window root;
XRectangle sgeo;
int screen;
Conf conf;
Key *keys;
Bool exiting;

/* Atoms / Cursors */
Atom wm_atom[WMLast];
Atom net_atom[NetLast];
Cursor cursor[CurLast];

/* Fonts */
XftFont *font;

/* InfoBar */
InfoBar *infobar;
Tag tags[MAXTAG];
int taglen[MAXTAG];
int seltag;

/* Important Client */
Client *clients;
Client *sel;
Client *selbytag[MAXTAG];

/* Other */
uint numlockmask;
uint scrolllockmask;
Variable confvar[256];

#endif /* WMFS_H */


