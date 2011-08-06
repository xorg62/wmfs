/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef EWMH_H
#define EWMH_H

#include "wmfs.h"

/* Ewmh hints list */
enum
{
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
     net_wm_system_tray_opcode,
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

void ewmh_init(void);

#endif /* EWMH_H */
