/*
*      wmfs.h
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
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

#define _BSD_SOURCE /* vsnprintf */
#define _POSIX_SOURCE /* kill */
/* Lib headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <dirent.h>
#include <err.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>

/* Local headers */
#include "parse/parse.h"
#include "config.h"
#include "structs.h"

/* Optional dependencies */
#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* HAVE_XINERAMA */

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif /* HAVE_XRANDR */

#ifdef HAVE_IMLIB
#include <Imlib2.h>
#endif /* HAVE_IMLIB */

/* MACRO */
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)
#define SCREEN       DefaultScreen(dpy)
#define ROOT         RootWindow(dpy, SCREEN)
#define MAXH         DisplayHeight(dpy, DefaultScreen(dpy))
#define MAXW         DisplayWidth(dpy, DefaultScreen(dpy))
#define INFOBARH     ((conf.bars.height > 0) ? conf.bars.height : (font->height * 1.5))
#define FHINFOBAR    ((font->height - font->descent) + (INFOBARH - font->height) / 2)
#define SHADH        (1)
#define SHADC        (0x000000) /* 'Cause i don't know how darken a color yet */
#define BORDH        conf.client.borderheight
#define TBARH        ((conf.titlebar.height < BORDH) ? BORDH : conf.titlebar.height)
#define RESHW        (6 * (BORDH))
#define BUTTONWH     (TBARH / 2)
#define DEF_CONF     ".config/wmfs/wmfsrc"
#define DEF_STATUS   ".config/wmfs/status.sh"
#define PAD          conf.pad
#define MAXSTATUS    (4096)

#define CWIN(win, parent, x, y, w, h, b, mask, col, at)                             \
    do {                                                                            \
        win = XCreateWindow(dpy, (parent), (x), (y), (w), (h), (b), CopyFromParent, \
        InputOutput, CopyFromParent, (mask), (at));                                 \
        XSetWindowBackground(dpy, win, (col));                                      \
    } while (/* CONSTCOND */ 0)

#define ATOM(a)      XInternAtom(dpy, (a), False)
#define FRAMEW(w)    ((w) + BORDH * 2)
#define FRAMEH(h)    ((h) + (BORDH  + TBARH))
#define ROUND(x)     (float)((x > 0) ? x + (float)0.5 : x - (float)0.5)
#define CHECK(x)     if(!(x)) return
#define IFREE(x)     if(x) free(x)
#define LEN(x)       (sizeof(x) / sizeof((x)[0]))
#define MAXCLIST     (64)

/* barwin.c */
BarWindow *barwin_create(Window parent,
                         int x, int y,
                         uint w, uint h,
                         uint bg, char*fg,
                         Bool entermask,
                         Bool stipple,
                         Bool border);
void barwin_draw_text(BarWindow *bw, int x, int y, char *text);
void barwin_delete(BarWindow *bw);
void barwin_delete_subwin(BarWindow *bw);
void barwin_map(BarWindow *bw);
void barwin_map_subwin(BarWindow *bw);
void barwin_unmap(BarWindow *bw);
void barwin_unmap_subwin(BarWindow *bw);
void barwin_move(BarWindow *bw, int x, int y);
void barwin_resize(BarWindow *bw, uint w, uint h);
void barwin_refresh_color(BarWindow *bw);
void barwin_refresh(BarWindow *bw);

/* draw.c */
void draw_text(Drawable d, int x, int y, char* fg, int pad, char *str);
void draw_rectangle(Drawable dr, int x, int y, uint w, uint h, uint color);
void draw_graph(Drawable dr, int x, int y, uint w, uint h, uint color, char *data);

#ifdef HAVE_IMLIB
void draw_image(Drawable dr, int x, int y, int w, int h, char *name);
#endif /* HAVE_IMLIB */

ushort textw(char *text);

/* infobar.c */
void infobar_init(void);
void infobar_draw(int sc);
void infobar_draw_layout(int sc);
void infobar_draw_selbar(int sc);
void infobar_draw_taglist(int sc);
void infobar_update_taglist(int sc);
void infobar_destroy(void);
void infobar_set_position(int pos);
void uicb_infobar_togglepos(uicb_t);
void uicb_toggle_tagautohide(uicb_t);

/* client.c */
void client_attach(Client *c);
void client_configure(Client *c);
void client_detach(Client *c);
void client_above(Client *c);
void client_focus(Client *c);
Client* client_get_next(void);
Client* client_get_prev(void);
Client* client_get_next_with_direction(Position pos);
/* client_gb_*() {{{ */
Client* client_gb_win(Window w);
Client* client_gb_frame(Window w);
Client* client_gb_titlebar(Window w);
Client* client_gb_resize(Window w);
Client* client_gb_button(Window w, int *n);
/* }}} */
void client_get_name(Client *c);
void client_hide(Client *c);
void client_kill(Client *c);
Bool ishide(Client *c, int screen);
void client_map(Client *c);
Client* client_manage(Window w, XWindowAttributes *wa, Bool ar);
void client_geo_hints(XRectangle *geo, Client *c);
void client_moveresize(Client *c, XRectangle geo, Bool r);
void client_maximize(Client *c);
void client_size_hints(Client *c);
void client_swap(Client *c1, Client *c2);
void client_raise(Client *c);
void client_unhide(Client *c);
void client_focus_next(Client *c);
void client_unmanage(Client *c);
void client_unmap(Client *c);
void client_set_rules(Client *c);
void client_update_attributes(Client *c);
void uicb_client_raise(uicb_t);
void uicb_client_next(uicb_t);
void uicb_client_prev(uicb_t);
void uicb_client_swap_next(uicb_t);
void uicb_client_swap_prev(uicb_t);
void uicb_client_focus_right(uicb_t cmd);
void uicb_client_focus_left(uicb_t cmd);
void uicb_client_focus_top(uicb_t cmd);
void uicb_client_focus_bottom(uicb_t cmd);
void uicb_client_kill(uicb_t);
void uicb_client_screen_next(uicb_t);
void uicb_client_screen_prev(uicb_t);
void uicb_client_move(uicb_t cmd);
void uicb_client_resize(uicb_t cmd);
void uicb_ignore_next_client_rules(uicb_t cmd);
void uicb_clientlist(uicb_t cmd);
void uicb_client_select(uicb_t cmd);
Bool uicb_checkclist(uicb_t);
void uicb_client_ignore_tag(uicb_t);

/* ewmh.c */
void ewmh_init_hints(void);
void ewmh_send_message(Window d, Window w, char *atom, long d0, long d1, long d2, long d3, long d4);
long ewmh_get_xembed_state(Window win);
void ewmh_get_number_of_desktop(void);
void ewmh_update_current_tag_prop(void);
void ewmh_get_client_list(void);
void ewmh_get_desktop_names(void);
void ewmh_set_desktop_geometry(void);
void ewmh_set_workarea(void);
void ewmh_manage_net_wm_state(long data_l[], Client *c);
void ewmh_manage_window_type(Client *c);

/* frame.c */
void frame_create(Client *c);
void frame_delete(Client *c);
void frame_moveresize(Client *c, XRectangle geo);
void frame_update(Client *c);

/* config.c */
void init_conf(void);

/* event.c */
void buttonpress(XButtonEvent *ev);
void configureevent(XConfigureRequestEvent *ev);
void clientmessageevent(XClientMessageEvent *ev);
void destroynotify(XDestroyWindowEvent *ev);
void enternotify(XCrossingEvent *ev);
void expose(XExposeEvent *ev);
void focusin(XFocusChangeEvent *ev);
void grabkeys(void);
void keypress(XKeyPressedEvent *ev);
void mappingnotify(XMappingEvent *ev);
void mapnotify(XMapEvent *ev);
void maprequest(XMapRequestEvent *ev);
void reparentnotify(XReparentEvent *ev);
void selectionclearevent(XSelectionClearEvent *ev);
void propertynotify(XPropertyEvent *ev);
void unmapnotify(XUnmapEvent *ev);
void getevent(XEvent ev);

/* menu.c */
void menu_init(Menu *menu, char *name, int nitem, uint bg_f, char *fg_f, uint bg_n, char *fg_n);
void menu_new_item(MenuItem *mi, char *name, void *func, char *cmd);
void menu_draw(Menu menu, int x, int y);
Bool menu_manage_event(XEvent *ev, Menu *menu, BarWindow *winitem[]);
Bool menu_activate_item(Menu *menu, int i);
void menu_focus_item(Menu *menu, int item, BarWindow *winitem[]);
void menu_draw_item_name(Menu *menu, int item, BarWindow *winitem[]);
int menu_get_longer_string(MenuItem *mi, int nitem);
void uicb_menu(uicb_t cmd);
void menu_clear(Menu *menu);

/* launcher.c */
void launcher_execute(Launcher *launcher);
void uicb_launcher(uicb_t);

/* mouse.c */
void mouse_dragborder(XRectangle geo, GC g);
void mouse_move_tile_client(Client **c);
void mouse_move_tag_client(Client *c);
void mouse_move(Client *c);
void mouse_resize(Client *c);
void mouse_grabbuttons(Client *c, Bool focused);
void uicb_mouse_move(uicb_t);
void uicb_mouse_resize(uicb_t);

/* util.c */
ulong color_enlight(ulong col);
void *emalloc(uint element, uint size);
long getcolor(char *color);
void setwinstate(Window win, long state);
char* _strdup(char const *str);
/* Conf usage {{{ */
void* name_to_func(char *name, const func_name_list_t *l);
ulong char_to_modkey(char *name, key_name_list_t key_l[]);
uint char_to_button(char *name, name_to_uint_t blist[]);
Layout layout_name_to_struct(Layout lt[], char *name, int n, const func_name_list_t llist[]);
char* alias_to_str(char *conf_choice);
/* }}} */
XRectangle get_mouse_pos(void);
char *char_to_str(const char c);
int spawn(const char *str, ...);
void swap_ptr(void **x, void **y);
void uicb_spawn(uicb_t);
char *clean_value(char *str);
char* patht(char *path);


#ifdef HAVE_IMLIB
int parse_image_block(ImageAttr *im, char *str);
#endif /* HAVE_IMLIB */

/* tag.c */
void tag_set(int tag);
void tag_transfert(Client *c, int tag);
void uicb_tag(uicb_t);
void uicb_tag_next(uicb_t);
void uicb_tag_prev(uicb_t);
void uicb_tag_next_visible(uicb_t);
void uicb_tag_prev_visible(uicb_t);
void uicb_tagtransfert(uicb_t);
void uicb_tag_prev_sel(uicb_t);
void uicb_tagtransfert_next(uicb_t);
void uicb_tagtransfert_prev(uicb_t);
void uicb_tag_urgent(uicb_t cmd);
void tag_additional(int sc, int tag, int adtag);
void uicb_tag_toggle_additional(uicb_t);
void tag_swap(int s, int t1, int t2);
void uicb_tag_swap(uicb_t);
void uicb_tag_swap_next(uicb_t);
void uicb_tag_swap_previous(uicb_t);
void tag_new(int s, char *name);
void uicb_tag_new(uicb_t);
void tag_delete(int s, int tag);
void uicb_tag_del(uicb_t);
void uicb_tag_rename(uicb_t cmd);

/* screen.c */
int screen_count(void);
XRectangle screen_get_geo(int s);
int screen_get_with_geo(int x, int y);
int screen_get_sel(void);
void screen_set_sel(int screen);
void screen_init_geo(void);
void uicb_screen_select(uicb_t);
void uicb_screen_next(uicb_t);
void uicb_screen_prev(uicb_t);
void uicb_screen_prev_sel(uicb_t);

/* status.c */
int statustext_rectangle(StatusRec *r, char *str);
int statustext_graph(StatusGraph *g, char *str);
int statustext_text(StatusText *s, char *str);
void statustext_normal(int sc, char *str);
void statustext_handle(int sc, char *str);

/* systray.c */
Bool systray_acquire(void);
void systray_add(Window win);
void systray_del(Systray *s);
void systray_state(Systray *s);
void systray_freeicons(void);
Systray* systray_find(Window win);
int systray_get_width(void);
void systray_update(void);

/* layout.c */
void arrange(int screen, Bool update_layout);
void freelayout(int screen);
void layoutswitch(Bool b);
void maxlayout(int screen);
Client *tiled_client(int screen, Client *c);
/* tile {{{ */
 void grid(int screen, Bool horizontal);
 void tile(int screen);
 void tile_left(int screen);
 void tile_top(int screen);
 void tile_bottom(int screen);
 void mirror_vertical(int screen);
 void mirror_horizontal(int screen);
 void layer(int screen);
 void grid_vertical(int screen);
 void grid_horizontal(int screen);
 /* }}} */
void uicb_togglemax(uicb_t);
void uicb_togglefree(uicb_t);
void uicb_layout_prev(uicb_t);
void uicb_layout_next(uicb_t);
void uicb_set_mwfact(uicb_t);
void uicb_set_nmaster(uicb_t);
void uicb_set_layout(uicb_t);
void uicb_toggle_resizehint(uicb_t);
void uicb_toggle_abovefc(uicb_t cmd);
void uicb_set_layer(uicb_t cmd);
void uicb_set_client_layer(uicb_t cmd);
void layout_set_client_master(Client *c);
Bool uicb_checkmax(uicb_t);
Bool uicb_checkfree(uicb_t);
Bool uicb_checklayout(uicb_t);

/* init.c */
void init(void);
void init_root(void);
void init_font(void);
void init_gc(void);
void init_cursor(void);
void init_key(void);
void init_geometry(void);
void init_status(void);

/* getinfo.c */
void getinfo_tag(void);
void getinfo_screen(void);
void getinfo_layout(void);
void getinfo_mwfact(void);
void getinfo_nmaster(void);
void getinfo(char *info);

/* viwmfs.c */
void viwmfs(int argc, char **argv);

/* wmfs.c */
int errorhandler(Display *d, XErrorEvent *event);
int errorhandlerdummy(Display *d, XErrorEvent *event);
void quit(void);
void *thread_process(void *arg);
void mainloop(void);
void scan(void);
Bool check_wmfs_running(void);
void exec_uicb_function(char *func, char *cmd);
void set_statustext(int s, char *str);
void update_status(void);
void handle_signal(int signum);
void uicb_quit(uicb_t);
void uicb_reload(uicb_t);

/* Variables */

/* Principal */
Display *dpy;
GC gc, gc_stipple;
int selscreen;
int prevselscreen;
Conf conf;
Key *keys;
Bool exiting, estatus;
XRectangle *sgeo;
XRectangle *spgeo;
Cursor cursor[CurLast];
char *argv_global;
int xrandr_event;
uint timing;

/* Fonts */
XftFont *font;

/* Atoms list */
Atom *net_atom;
Atom trayatom;

/* InfoBar/Tags */
InfoBar *infobar;
Tag **tags;
int *seltag;
int *prevseltag;
Menu menulayout;

/* ClientList */
Menu clientlist;
struct clndx {
     char key[4];
     Client *client;
} clist_index[MAXCLIST];

/* Important Client */
Client *clients;
Client *sel;

/* Other */
func_name_list_t *func_list;
extern const func_name_list_t layout_list[];
uint numlockmask;
Systray *trayicons;
Window traywin;
int tray_width;

#endif /* WMFS_H */


