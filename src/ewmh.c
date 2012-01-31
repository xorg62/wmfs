/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "ewmh.h"
#include "util.h"
#include "screen.h"
#include "client.h"

/* Taken From standards.freedesktop.org */
#define _NET_WM_STATE_REMOVE 0 /* remove/unset property */
#define _NET_WM_STATE_ADD    1 /* add/set property */
#define _NET_WM_STATE_TOGGLE 2 /* toggle property  */

void
ewmh_init(void)
{
     int b = 1;

     W->net_atom = xcalloc(net_last, sizeof(Atom));

     /* EWMH hints */
     W->net_atom[wm_state]                       = ATOM("WM_STATE");
     W->net_atom[wm_class]                       = ATOM("WM_CLASS");
     W->net_atom[net_supported]                  = ATOM("_NET_SUPPORTED");
     W->net_atom[net_client_list]                = ATOM("_NET_CLIENT_LIST");
     W->net_atom[net_frame_extents]              = ATOM("_NET_FRAME_EXTENTS");
     W->net_atom[net_number_of_desktops]         = ATOM("_NET_NUMBER_OF_DESKTOPS");
     W->net_atom[net_current_desktop]            = ATOM("_NET_CURRENT_DESKTOP");
     W->net_atom[net_desktop_names]              = ATOM("_NET_DESKTOP_NAMES");
     W->net_atom[net_desktop_geometry]           = ATOM("_NET_DESKTOP_GEOMETRY");
     W->net_atom[net_active_window]              = ATOM("_NET_ACTIVE_WINDOW");
     W->net_atom[net_close_window]               = ATOM("_NET_CLOSE_WINDOW");
     W->net_atom[net_wm_name]                    = ATOM("_NET_WM_NAME");
     W->net_atom[net_wm_pid]                     = ATOM("_NET_WM_PID");
     W->net_atom[net_wm_desktop]                 = ATOM("_NET_WM_DESKTOP");
     W->net_atom[net_showing_desktop]            = ATOM("_NET_SHOWING_DESKTOP");
     W->net_atom[net_wm_icon_name]               = ATOM("_NET_WM_ICON_NAME");
     W->net_atom[net_wm_window_type]             = ATOM("_NET_WM_WINDOW_TYPE");
     W->net_atom[net_supporting_wm_check]        = ATOM("_NET_SUPPORTING_WM_CHECK");
     W->net_atom[net_wm_window_opacity]          = ATOM("_NET_WM_WINDOW_OPACITY");
     W->net_atom[net_wm_window_type_normal]      = ATOM("_NET_WM_WINDOW_TYPE_NORMAL");
     W->net_atom[net_wm_window_type_dock]        = ATOM("_NET_WM_WINDOW_TYPE_DOCK");
     W->net_atom[net_wm_window_type_splash]      = ATOM("_NET_WM_WINDOW_TYPE_SPLASH");
     W->net_atom[net_wm_window_type_dialog]      = ATOM("_NET_WM_WINDOW_TYPE_DIALOG");
     W->net_atom[net_wm_icon]                    = ATOM("_NET_WM_ICON");
     W->net_atom[net_wm_state]                   = ATOM("_NET_WM_STATE");
     W->net_atom[net_wm_state_fullscreen]        = ATOM("_NET_WM_STATE_FULLSCREEN");
     W->net_atom[net_wm_state_sticky]            = ATOM("_NET_WM_STATE_STICKY");
     W->net_atom[net_wm_state_demands_attention] = ATOM("_NET_WM_STATE_DEMANDS_ATTENTION");
     W->net_atom[net_wm_state_hidden]            = ATOM("_NET_WM_STATE_HIDDEN");
     W->net_atom[net_system_tray_s]              = ATOM("_NET_SYSTEM_TRAY_S0");
     W->net_atom[net_system_tray_opcode]         = ATOM("_NET_SYSTEM_TRAY_OPCODE");
     W->net_atom[net_system_tray_message_data]   = ATOM("_NET_SYSTEM_TRAY_MESSAGE_DATA");
     W->net_atom[net_system_tray_visual]         = ATOM("_NET_SYSTEM_TRAY_VISUAL");
     W->net_atom[net_system_tray_orientation]    = ATOM("_NET_SYSTEM_TRAY_ORIENTATION");
     W->net_atom[xembed]                         = ATOM("_XEMBED");
     W->net_atom[xembedinfo]                     = ATOM("_XEMBED_INFO");
     W->net_atom[manager]                        = ATOM("MANAGER");
     W->net_atom[utf8_string]                    = ATOM("UTF8_STRING");

     /* WMFS hints */
     W->net_atom[wmfs_running]                   = ATOM("_WMFS_RUNNING");
     W->net_atom[wmfs_focus]                     = ATOM("_WMFS_FOCUS");
     W->net_atom[wmfs_update_hints]              = ATOM("_WMFS_UPDATE_HINTS");
     W->net_atom[wmfs_set_screen]                = ATOM("_WMFS_SET_SCREEN");
     W->net_atom[wmfs_screen_count]              = ATOM("_WMFS_SCREEN_COUNT");
     W->net_atom[wmfs_current_tag]               = ATOM("_WMFS_CURRENT_TAG");
     W->net_atom[wmfs_tag_list]                  = ATOM("_WMFS_TAG_LIST");
     W->net_atom[wmfs_current_screen]            = ATOM("_WMFS_CURRENT_SCREEN");
     W->net_atom[wmfs_current_layout]            = ATOM("_WMFS_CURRENT_LAYOUT");
     W->net_atom[wmfs_function]                  = ATOM("_WMFS_FUNCTION");
     W->net_atom[wmfs_cmd]                       = ATOM("_WMFS_CMD");

     XChangeProperty(W->dpy, W->root, W->net_atom[net_supported], XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)W->net_atom, net_last);

     XChangeProperty(W->dpy, W->root, W->net_atom[wmfs_running], XA_CARDINAL, 32,
                     PropModeReplace, (unsigned char*)&b, 1);

     /* Set _NET_SUPPORTING_WM_CHECK */
     XChangeProperty(W->dpy, W->root, W->net_atom[net_supporting_wm_check], XA_WINDOW, 32,
                     PropModeReplace, (unsigned char*)&W->root, 1);

     XChangeProperty(W->dpy, W->root, ATOM("WM_CLASS"), XA_STRING, 8,
                     PropModeReplace, (unsigned char*)&"wmfs", 4);

     XChangeProperty(W->dpy, W->root, W->net_atom[net_wm_name], W->net_atom[utf8_string], 8,
                     PropModeReplace, (unsigned char*)&"wmfs2", 5);

         /*

      * Set _NET_WM_PID
     XChangeProperty(W->dpy, W->root, W->net_atom[net_wm_pid], XA_CARDINAL, 32,
                     PropModeReplace, (unsigned char*)&pid, 1);

      * Set _NET_SHOWING_DESKTOP
     XChangeProperty(W->dpy, W->root, W->net_atom[net_showing_desktop], XA_CARDINAL, 32,
                     PropModeReplace, (unsigned char*)&showing_desk, 1);
      */

}

void
ewmh_set_wm_state(Window w, int state)
{
     unsigned char d[] = { state, None };

     XChangeProperty(W->dpy, w, W->net_atom[wm_state],
                     W->net_atom[wm_state], 32, PropModeReplace, d, 2);
}

/*
 * Get xembed state
 */
long
ewmh_get_xembed_state(Window win)
{
     Atom rf;
     int f;
     long ret = 0;
     unsigned long n, il;
     unsigned char *data = NULL;

     if(XGetWindowProperty(W->dpy, win, W->net_atom[xembedinfo], 0L, 2, False,
                           W->net_atom[xembedinfo], &rf, &f, &n, &il, &data) != Success)
          return 0;

     if(rf == W->net_atom[xembedinfo] && n == 2)
          ret = (long)data[1];

     if(n && data)
          XFree(data);

     return ret;
}

void
ewmh_update_wmfs_props(void)
{
     struct screen *s;
     int i, ns = 0;
     long *cts = NULL;

     SLIST_FOREACH(s, &W->h.screen, next)
          ++ns;

     cts = xcalloc(ns, sizeof(long));

     for(i = 0; i < ns; ++i)
     {
          s = screen_gb_id(i);
          cts[i] = (s->seltag ? s->seltag->id : 0);
     }

     XChangeProperty(W->dpy, W->root, W->net_atom[wmfs_current_tag], XA_CARDINAL, 32,
                     PropModeReplace, (unsigned char*)cts, ns);

     if(W->client)
          XChangeProperty(W->dpy, W->root, W->net_atom[wmfs_focus], XA_WINDOW, 32,
                          PropModeReplace, (unsigned char*)&W->client->win, 1);

     free(cts);
}

void
ewmh_manage_state(long data[], struct client *c)
{
     /* _NET_WM_STATE_FULLSCREEN */
     if(data[1] == (long)W->net_atom[net_wm_state_fullscreen])
     {
          if(data[0] == _NET_WM_STATE_ADD
             || (data[0] == _NET_WM_STATE_TOGGLE && !(c->flags & CLIENT_FULLSCREEN)))
          {
               c->flags |= CLIENT_FULLSCREEN;

               XReparentWindow(W->dpy, c->win, W->root, c->screen->geo.x, c->screen->geo.y);
               XResizeWindow(W->dpy, c->win, c->screen->geo.w, c->screen->geo.h);
               XChangeProperty(W->dpy, c->win, W->net_atom[net_wm_state], XA_ATOM, 32, PropModeReplace,
                               (unsigned char*)&W->net_atom[net_wm_state_fullscreen], true);

               client_focus(c);

               XRaiseWindow(W->dpy, c->win);
          }
          else if(data[0] == _NET_WM_STATE_REMOVE
                  || (data[0] == _NET_WM_STATE_TOGGLE && c->flags & CLIENT_FULLSCREEN))
          {
               c->flags &= ~CLIENT_FULLSCREEN;

               XReparentWindow(W->dpy, c->win, c->frame, c->wgeo.x, c->wgeo.y);
               XChangeProperty(W->dpy, c->win, W->net_atom[net_wm_state], XA_ATOM, 32, PropModeReplace,
                               (unsigned char*)&W->net_atom[net_wm_state_fullscreen], false);

               client_moveresize(c, &c->geo);
          }
     }
}

void
ewmh_manage_window_type(struct client *c)
{
     Atom *atom, rf;
     int f;
     unsigned long n, il, i;
     unsigned char *data = NULL;
     long ldata[5] = { _NET_WM_STATE_ADD };

     /* _NET_WM_STATE at window mangement */
     if(XGetWindowProperty(W->dpy, c->win, W->net_atom[net_wm_state], 0L, 0x7FFFFFFFL, false,
                           XA_ATOM, &rf, &f, &n, &il, &data) == Success && n)
     {
          atom = (Atom*)data;

          for(i = 0; i < n; ++i)
          {
               ldata[1] = atom[i];
               ewmh_manage_state(ldata, c);
          }

          XFree(data);
     }
}

