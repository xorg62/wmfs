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

/* Took From standards.freedesktop.org */
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

/** Init ewmh atoms
*/
void
ewmh_init_hints(void)
{
     net_atom[net_supported]                  = ATOM("_NET_SUPPORTED");
     net_atom[net_client_list]                = ATOM("_NET_CLIENT_LIST");
     net_atom[net_number_of_desktops]         = ATOM("_NET_NUMBER_OF_DESKTOPS");
     net_atom[net_current_desktop]            = ATOM("_NET_CURRENT_DESKTOP");
     net_atom[net_desktop_names]              = ATOM("_NET_DESKTOP_NAMES");
     net_atom[net_desktop_names_string]       = ATOM("_NET_DESKTOP_NAMES_STRING");
     net_atom[net_desktop_geometry]           = ATOM("_NET_DESKTOP_GEOMETRY");
     net_atom[net_workarea]                   = ATOM("_NET_WORKAREA");
     net_atom[net_active_window]              = ATOM("_NET_ACTIVE_WINDOW");
     net_atom[net_close_window]               = ATOM("_NET_CLOSE_WINDOW");
     net_atom[net_wm_name]                    = ATOM("_NET_WM_NAME");
     net_atom[net_wm_icon_name]               = ATOM("_NET_WM_ICON_NAME");
     net_atom[net_wm_window_type]             = ATOM("_NET_WM_WINDOW_TYPE");
     net_atom[net_wm_window_type_normal]      = ATOM("_NET_WM_WINDOW_TYPE_NORMAL");
     net_atom[net_wm_window_type_dock]        = ATOM("_NET_WM_WINDOW_TYPE_DOCK");
     net_atom[net_wm_window_type_splash]      = ATOM("_NET_WM_WINDOW_TYPE_SPLASH");
     net_atom[net_wm_window_type_dialog]      = ATOM("_NET_WM_WINDOW_TYPE_DIALOG");
     net_atom[net_wm_icon]                    = ATOM("_NET_WM_ICON");
     net_atom[net_wm_state]                   = ATOM("_NET_WM_STATE");
     net_atom[net_wm_state_sticky]            = ATOM("_NET_WM_STATE_STICKY");
     net_atom[net_wm_state_skip_taskbar]      = ATOM("_NET_WM_STATE_SKIP_TASKBAR");
     net_atom[net_wm_state_fullscreen]        = ATOM("_NET_WM_STATE_FULLSCREEN");
     net_atom[net_wm_state_demands_attention] = ATOM("_NET_WM_STATE_DEMANDS_ATTENTION");
     net_atom[utf8_string]                    = ATOM("UTF8_STRING");

     XChangeProperty(dpy, ROOT, net_atom[net_supported], XA_ATOM, 32,
                     PropModeReplace, (uchar*)net_atom, net_last);

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
ewmh_get_current_desktop(void)
{
     int t;

     screen_get_sel();
     t = seltag[selscreen] - 1;

     /* Get current desktop (tag) */
     XChangeProperty(dpy, ROOT, net_atom[net_current_desktop], XA_CARDINAL, 32,
                     PropModeReplace, (uchar*)&t, 1);

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

     XChangeProperty(dpy, ROOT, net_atom[net_desktop_names_string], XA_STRING, 8,
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
               data[pos++] = sgeo[i].x - BORDH;
               data[pos++] = sgeo[i].y - TBARH;
               data[pos++] = sgeo[i].width;
               data[pos++] = sgeo[i].height;
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
               c->state_fullscreen = True;
               c->tmp_geo = c->geo;
               if(c->free)
                    c->ogeo = c->geo;
               client_maximize(c);
          }
          else if(data_l[0] == _NET_WM_STATE_REMOVE && c->state_fullscreen)
          {
               c->state_fullscreen = False;
               client_moveresize(c, c->tmp_geo, False);
               tags[selscreen][seltag[selscreen]].layout.func();
          }
     }
     /* Manage _NET_WM_STATE_DEMANDS_ATTENTION */
     else if(data_l[1] == net_atom[net_wm_state_demands_attention])
     {
          if(data_l[0] == _NET_WM_STATE_ADD)
          {
               screen_set_sel(c->screen);
               tag_set(c->tag);
               client_focus(c);
          }
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
                    XReparentWindow(dpy, c->win, ROOT, c->geo.x, c->geo.y);
                    XUnmapSubwindows(dpy, c->frame);
                    XUnmapWindow(dpy, c->frame);
                    XRaiseWindow(dpy, c->win);
                    c->state_dock = True;
               }
               /* MANAGE _NET_WM_WINDOW_TYPE_DIALOG */
               else if(atom[i] == net_atom[net_wm_window_type_dialog])
               {
                    c->free = True;
                    sel->tile = sel->max = sel->lmax = False;
                    client_moveresize(sel, sel->ogeo, True);
                    client_focus(c);
                    tags[selscreen][seltag[selscreen]].layout.func();
               }
          }
          XFree(data);
     }

     return;
}


/** Get a Window WM State
 * \param win Window
 * \return The state
*/
long
ewmh_get_wm_state(Window win)
{
     Atom rt;
     int rf;
     long ret = WithdrawnState;
     ulong ir, il;
     uchar *data;

     if(XGetWindowProperty(dpy, win, net_atom[net_wm_state], 0L, 2L,
                           False, net_atom[net_wm_state], &rt,
                           &rf, &ir, &il, &data) == Success && ir)
     {
          ret = *(long *)data;
          XFree(data);
     }

     return ret;
}
