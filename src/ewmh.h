/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef EWMH_H
#define EWMH_H

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "wmfs.h"
#include "util.h"

/* EWMH/Xembed const from freedesktop */
#define XEMBED_MAPPED                 (1 << 0)
#define XEMBED_EMBEDDED_NOTIFY        0
#define XEMBED_WINDOW_ACTIVATE        1
#define XEMBED_WINDOW_DEACTIVATE      2
#define XEMBED_REQUEST_FOCUS          3
#define XEMBED_FOCUS_IN               4
#define XEMBED_FOCUS_OUT              5
#define XEMBED_FOCUS_NEXT             6
#define XEMBED_FOCUS_PREV             7
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON            10
#define XEMBED_MODALITY_OFF           11
#define XEMBED_REGISTER_ACCELERATOR   12
#define XEMBED_UNREGISTER_ACCELERATOR 13
#define XEMBED_ACTIVATE_ACCELERATOR   14

/* Details for  XEMBED_FOCUS_IN: */
#define XEMBED_FOCUS_CURRENT 0
#define XEMBED_FOCUS_FIRST   1
#define XEMBED_FOCUS_LAST    2

/* Ewmh hints list */
enum
{
     /* ICCCM */
     wm_state,
     wm_class,
     /* EWMH */
     net_supported,
     net_wm_name,
     net_client_list,
     net_frame_extents,
     net_number_of_desktops,
     net_current_desktop,
     net_desktop_names,
     net_desktop_geometry,
     net_active_window,
     net_close_window,
     net_wm_icon_name,
     net_wm_window_type,
     net_wm_pid,
     net_showing_desktop,
     net_supporting_wm_check,
     net_wm_window_opacity,
     net_wm_window_type_normal,
     net_wm_window_type_dock,
     net_wm_window_type_splash,
     net_wm_window_type_dialog,
     net_wm_desktop,
     net_wm_icon,
     net_wm_state,
     net_wm_state_fullscreen,
     net_wm_state_sticky,
     net_wm_state_demands_attention,
     net_wm_state_hidden,
     net_system_tray_opcode,
     net_system_tray_message_data,
     net_system_tray_s,
     net_system_tray_visual,
     net_system_tray_orientation,
     xembed,
     xembedinfo,
     manager,
     utf8_string,
     /* WMFS HINTS */
     wmfs_running,
     wmfs_focus,
     wmfs_update_hints,
     wmfs_current_tag,
     wmfs_current_screen,
     wmfs_current_layout,
     wmfs_tag_list,
     wmfs_mwfact,
     wmfs_nmaster,
     wmfs_set_screen,
     wmfs_screen_count,
     wmfs_function,
     wmfs_cmd,
     wmfs_font,
     wmfs_statustext,
     net_last
};

static inline void
ewmh_send_message(Window d, Window w, char *atom, long d0, long d1, long d2, long d3, long d4)
{
     XClientMessageEvent e;

     e.type          = ClientMessage;
     e.message_type  = ATOM(atom);
     e.window        = w;
     e.format        = 32;
     e.data.l[0]     = d0;
     e.data.l[1]     = d1;
     e.data.l[2]     = d2;
     e.data.l[3]     = d3;
     e.data.l[4]     = d4;

     XSendEvent(W->dpy, d, false, StructureNotifyMask, (XEvent*)&e);
     XSync(W->dpy, False);
}

void ewmh_init(void);
void ewmh_set_wm_state(Window w, int state);
void ewmh_get_client_list(void);
long ewmh_get_xembed_state(Window win);
void ewmh_update_wmfs_props(void);
void ewmh_manage_state(long data[], struct client *c);
void ewmh_manage_window_type(struct client *c);

#endif /* EWMH_H */
