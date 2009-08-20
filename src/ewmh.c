/*
*      ewmh.c
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

#include "wmfs.h"

/* Taken From standards.freedesktop.org */
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

/** Init ewmh atoms
*/
void
ewmh_init_hints(void)
{
     int i = 1, showing_desk = 0;
     char root_name[] = "WMFS "WMFS_VERSION;
     char class[] = "wmfs";
     long pid = (long)getpid();

     /* EWMH hints */
     net_atom[net_supported]                  = ATOM("_NET_SUPPORTED");
     net_atom[net_client_list]                = ATOM("_NET_CLIENT_LIST");
     net_atom[net_frame_extents]              = ATOM("_NET_FRAME_EXTENTS");
     net_atom[net_number_of_desktops]         = ATOM("_NET_NUMBER_OF_DESKTOPS");
     net_atom[net_current_desktop]            = ATOM("_NET_CURRENT_DESKTOP");
     net_atom[net_desktop_names]              = ATOM("_NET_DESKTOP_NAMES");
     net_atom[net_desktop_geometry]           = ATOM("_NET_DESKTOP_GEOMETRY");
     net_atom[net_workarea]                   = ATOM("_NET_WORKAREA");
     net_atom[net_active_window]              = ATOM("_NET_ACTIVE_WINDOW");
     net_atom[net_close_window]               = ATOM("_NET_CLOSE_WINDOW");
     net_atom[net_wm_name]                    = ATOM("_NET_WM_NAME");
     net_atom[net_wm_pid]                     = ATOM("_NET_WM_PID");
     net_atom[net_wm_desktop]                 = ATOM("_NET_WM_DESKTOP");
     net_atom[net_showing_desktop]            = ATOM("_NET_SHOWING_DESKTOP");
     net_atom[net_wm_icon_name]               = ATOM("_NET_WM_ICON_NAME");
     net_atom[net_wm_window_type]             = ATOM("_NET_WM_WINDOW_TYPE");
     net_atom[net_supporting_wm_check]        = ATOM("_NET_SUPPORTING_WM_CHECK");
     net_atom[net_wm_window_type_normal]      = ATOM("_NET_WM_WINDOW_TYPE_NORMAL");
     net_atom[net_wm_window_type_dock]        = ATOM("_NET_WM_WINDOW_TYPE_DOCK");
     net_atom[net_wm_window_type_splash]      = ATOM("_NET_WM_WINDOW_TYPE_SPLASH");
     net_atom[net_wm_window_type_dialog]      = ATOM("_NET_WM_WINDOW_TYPE_DIALOG");
     net_atom[net_wm_icon]                    = ATOM("_NET_WM_ICON");
     net_atom[net_wm_state]                   = ATOM("_NET_WM_STATE");
     net_atom[net_wm_state_fullscreen]        = ATOM("_NET_WM_STATE_FULLSCREEN");
     net_atom[net_wm_state_demands_attention] = ATOM("_NET_WM_STATE_DEMANDS_ATTENTION");
     net_atom[utf8_string]                    = ATOM("UTF8_STRING");

     /* WMFS hints */
     net_atom[wmfs_running]                   = ATOM("_WMFS_RUNNING");
     net_atom[wmfs_update_hints]              = ATOM("_WMFS_UPDATE_HINTS");
     net_atom[wmfs_statustext]                = ATOM("_WMFS_STATUSTEXT");
     net_atom[wmfs_set_screen]                = ATOM("_WMFS_SET_SCREEN");
     net_atom[wmfs_screen_count]              = ATOM("_WMFS_SCREEN_COUNT");
     net_atom[wmfs_current_tag]               = ATOM("_WMFS_CURRENT_TAG");
     net_atom[wmfs_tag_list]                  = ATOM("_WMFS_TAG_LIST");
     net_atom[wmfs_current_screen]            = ATOM("_WMFS_CURRENT_SCREEN");
     net_atom[wmfs_current_layout]            = ATOM("_WMFS_CURRENT_LAYOUT");
     net_atom[wmfs_mwfact]                    = ATOM("_WMFS_MWFACT");
     net_atom[wmfs_nmaster]                   = ATOM("_WMFS_NMASTER");
     net_atom[wmfs_function]                  = ATOM("_WMFS_FUNCTION");
     net_atom[wmfs_cmd]                       = ATOM("_WMFS_CMD");

     XChangeProperty(dpy, ROOT, net_atom[net_supported], XA_ATOM, 32,
                     PropModeReplace, (uchar*)net_atom, net_last);

     XChangeProperty(dpy, ROOT, net_atom[wmfs_running], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&i, 1);

     /* Set _NET_SUPPORTING_WM_CHECK */
     XChangeProperty(dpy, ROOT, net_atom[net_supporting_wm_check], XA_WINDOW, 32,
                     PropModeReplace, (uchar*)&ROOT, 1);

     XChangeProperty(dpy, ROOT, net_atom[net_wm_name], net_atom[utf8_string], 8,
                     PropModeReplace, (uchar*)&root_name, strlen(root_name));

     XChangeProperty(dpy, ROOT, ATOM("WM_CLASS"), XA_STRING, 8,
                     PropModeReplace, (uchar*)&class, strlen(class));

     /* Set _NET_WM_PID */
     XChangeProperty(dpy, ROOT, net_atom[net_wm_pid], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&pid, 1);

     /* Set _NET_SHOWING_DESKTOP */
     XChangeProperty(dpy, ROOT, net_atom[net_showing_desktop], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&showing_desk, 1);

     return;
}

/** Get the number of desktop (tag)
*/
void
ewmh_get_number_of_desktop(void)
{
     int c = 0, i;

     for(i = 0; i < screen_count(); ++i)
          c += conf.ntag[i];

     XChangeProperty(dpy, ROOT, net_atom[net_number_of_desktops], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&c, 1);

     return;
}

/** Get the current desktop
*/
void
ewmh_update_current_tag_prop(void)
{
     int t;
     char *s = NULL;

     screen_get_sel();
     t = seltag[selscreen] - 1;

     s = emalloc(8, sizeof(char));

     /* Get current desktop (tag) */
     XChangeProperty(dpy, ROOT, net_atom[net_current_desktop], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&t, 1);

     /* Current tag name */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_current_tag], net_atom[utf8_string], 8,
                     PropModeReplace, (uchar*)tags[selscreen][seltag[selscreen]].name,
                     strlen(tags[selscreen][seltag[selscreen]].name));

     sprintf(s, "%.3f", tags[selscreen][t + 1].mwfact);

     /* Current tag mwfact */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_mwfact], XA_STRING, 8,
                     PropModeReplace, (uchar*)s, strlen(s));

     /* Current nmaster */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_nmaster], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&tags[selscreen][t + 1].nmaster, 1);

     /* Current layout */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_current_layout], net_atom[utf8_string], 8,
                     PropModeReplace, (uchar*)tags[selscreen][seltag[selscreen]].layout.symbol,
                     strlen(tags[selscreen][seltag[selscreen]].layout.symbol));

     free(s);

     return;
}

/** Get _NET_CLIENT_LIST
*/
void
ewmh_get_client_list(void)
{
     Window *list;
     Client *c;
     int win_n;

     for(win_n = 0, c = clients; c; c = c->next, ++win_n);
     list = emalloc(win_n, sizeof(Window));

     for(win_n = 0, c = clients; c; c = c->next, ++win_n)
          list[win_n] = c->win;

     XChangeProperty(dpy, ROOT, net_atom[net_client_list], XA_WINDOW, 32,
                     PropModeReplace, (uchar *)list, win_n);

     free(list);

     return;
}

/** The desktop names
 */
void
ewmh_get_desktop_names(void)
{
     char *str = NULL;
     int s, i, len = 0, pos = 0;

     for(s = 0 ; s < screen_count(); ++s)
          for(i = 1; i < conf.ntag[s] + 1; ++i)
               len += strlen(tags[s][i].name);

     str = emalloc(len + i + 1, sizeof(char*));

     for(s = 0; s < screen_count(); ++s)
          for(i = 1; i < conf.ntag[s] + 1; ++i, ++pos)
          {
               strncpy(str + pos, tags[s][i].name, strlen(tags[s][i].name));
               pos += strlen(tags[s][i].name);
               str[pos] = '\0';
          }

     XChangeProperty(dpy, ROOT, net_atom[net_desktop_names], net_atom[utf8_string], 8,
                     PropModeReplace, (uchar*)str, pos);

     for(i = 0; i < pos; ++i)
          if(str[i] == '\0' && i < pos - 1)
               str[i] = ' ';

     XChangeProperty(dpy, ROOT, net_atom[wmfs_tag_list], net_atom[utf8_string], 8,
                     PropModeReplace, (uchar*)str, pos);

     free(str);

     return;
}


/** Manage _NET_DESKTOP_GEOMETRY
*/
void
ewmh_set_desktop_geometry(void)
{
     long data[2] = { MAXW, MAXH };

     XChangeProperty(dpy, ROOT, net_atom[net_desktop_geometry], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&data, 2);

     return;
}

/** Manage _NET_WORKAREA
*/
void
ewmh_set_workarea(void)
{
     long *data;
     int i, j, tag_c = 0, pos = 0;

     for(i = 0; i < screen_count(); ++i)
          tag_c += conf.ntag[i];

     data = emalloc(tag_c * 4, sizeof(long));

     for(i = 0; i < screen_count(); ++i)
          for(j = 0; j < conf.ntag[i]; ++j)
          {
               data[pos++] = spgeo[i].x;
               data[pos++] = spgeo[i].y;
               data[pos++] = spgeo[i].width;
               data[pos++] = spgeo[i].height;
          }

     XChangeProperty(dpy, ROOT, net_atom[net_workarea], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)data, 4 * tag_c);

     free(data);

     return;
}

/** Manage _NET_WM_STATE_* ewmh
 */
void
ewmh_manage_net_wm_state(long data_l[], Client *c)
{
     /* Manage _NET_WM_STATE_FULLSCREEN */
     if(data_l[1] == net_atom[net_wm_state_fullscreen])
     {
          if(data_l[0] == _NET_WM_STATE_ADD && !c->state_fullscreen)
          {
               c->screen = screen_get_with_geo(c->geo.x, c->geo.y);
               client_unmap(c);
               c->unmapped = False;
               XMapWindow(dpy, c->win);
               XReparentWindow(dpy, c->win, ROOT, spgeo[c->screen].x, spgeo[c->screen].y);
               XResizeWindow(dpy, c->win, spgeo[c->screen].x + spgeo[c->screen].width,
                             spgeo[c->screen].y + spgeo[c->screen].height);
               c->tmp_geo = c->geo;

               if(c->free)
                    c->ogeo = c->geo;

               c->state_fullscreen = True;
               c->max = True;

               client_raise(c);
               client_focus(c);
          }
          else if(data_l[0] == _NET_WM_STATE_REMOVE && c->state_fullscreen)
          {
               c->state_fullscreen = False;
               client_map(c);
               XReparentWindow(dpy, c->win, c->frame, BORDH, TBARH);
               client_moveresize(c, c->tmp_geo, False);
          }
     }
     /* Manage _NET_WM_STATE_DEMANDS_ATTENTION */
     else if(data_l[1] == net_atom[net_wm_state_demands_attention])
     {
          if(data_l[0] == _NET_WM_STATE_ADD)
               client_focus(c);
          if(data_l[0] == _NET_WM_STATE_REMOVE)
               if(c == sel)
                    client_focus(NULL);
     }

     return;
}


/** Manage the client hints
 *\param c Client pointer
*/
void
ewmh_manage_window_type(Client *c)
{
     Atom *atom, rf;
     int i, f;
     ulong n, il;
     uchar *data = NULL;

     if(XGetWindowProperty(dpy, c->win, net_atom[net_wm_window_type], 0L, 0x7FFFFFFFL,
                           False, XA_ATOM, &rf, &f, &n, &il, &data) == Success && n)
     {
          atom = (Atom*)data;

          for(i = 0; i < n; ++i)
          {
               /* Manage _NET_WM_WINDOW_TYPE_DOCK & _NET_WM_WINDOW_TYPE_SPLASH */
               if(atom[i] == net_atom[net_wm_window_type_dock]
                  || atom[i] == net_atom[net_wm_window_type_splash])
               {
                    /* Unmap frame, decoration.. */
                    client_unmap(c);

                    /* Map only window */
                    XMapWindow(dpy, c->win);

                    /* Reparent it to ROOT win */
                    XReparentWindow(dpy, c->win, ROOT, c->geo.x, c->geo.y);
                    XRaiseWindow(dpy, c->win);

                    /* This window will not be managed anymore,
                     * so let's detach it. */
                    client_detach(c);
               }
               /* MANAGE _NET_WM_WINDOW_TYPE_DIALOG */
               else if(atom[i] == net_atom[net_wm_window_type_dialog])
               {
                    c->free = True;
                    sel->tile = sel->max = sel->lmax = False;
                    client_moveresize(sel, sel->ogeo, True);
                    client_focus(c);
                    tags[selscreen][seltag[selscreen]].layout.func(selscreen);
               }
          }
          XFree(data);
     }

     return;
}

