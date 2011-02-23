/*
*      event.c
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

#include "wmfs.h"

/** ButtonPress handle event
 * \param ev XButtonEvent pointer
*/
static void
buttonpress(XButtonEvent *ev)
{
     Client *c;
     int i, j, n;

     screen_get_sel();

     /* If the mouse is on a not selected client and you click on it. */
     if(((c = client_gb_win(ev->window)) || (c = client_gb_titlebar(ev->window))) && c != sel
        && (ev->button == Button1 || ev->button == Button2 || ev->button == Button3))
     {
          client_focus(c);
          client_raise(c);

          return;
     }

     /* Titlebar */
     if((c = client_gb_titlebar(ev->window)) && c == sel)
          for(i = 0; i < conf.titlebar.nmouse; ++i)
                if(ev->button == conf.titlebar.mouse[i].button)
                    if(conf.titlebar.mouse[i].func)
                         conf.titlebar.mouse[i].func(conf.titlebar.mouse[i].cmd);

     /* Titlebar buttons */
     if((c = client_gb_button(ev->window, &n)))
          for(i = 0; i < conf.titlebar.button[n].nmouse; ++i)
               if(ev->button == conf.titlebar.button[n].mouse[i].button)
                    if(conf.titlebar.button[n].mouse[i].func)
                    {
                         client_focus(c);
                         conf.titlebar.button[n].mouse[i].func(conf.titlebar.button[n].mouse[i].cmd);
                    }

     /* Frame Resize Area */
     if((c = client_gb_resize(ev->window)))
          mouse_resize(c);

     /* Client */
     if((c = client_gb_win(ev->window)) && c == sel)
          for(i = 0; i < conf.client.nmouse; ++i)
               if(ev->button == conf.client.mouse[i].button)
                    if(conf.client.mouse[i].func)
                         conf.client.mouse[i].func(conf.client.mouse[i].cmd);

     /* Root */
     if(ev->window == ROOT)
          for(i = 0; i < conf.root.nmouse; ++i)
               if(conf.root.mouse[i].tag == seltag[conf.root.mouse[i].screen]
                  || conf.root.mouse[i].tag < 0)
                    if(ev->button == conf.root.mouse[i].button)
                         if(conf.root.mouse[i].func)
                              conf.root.mouse[i].func(conf.root.mouse[i].cmd);

     /* Infobars */
     for(i = 0; i < screen_count(); ++i)
          if(ev->window == infobar[i].bar->win)
               for(j = 0; j < conf.bars.nmouse; ++j)
                    if(conf.bars.mouse[j].screen == i
                       || conf.bars.mouse[j].screen < 0)
                         if(conf.bars.mouse[j].tag == seltag[i]
                            || conf.bars.mouse[j].tag < 0)
                              if(ev->button == conf.bars.mouse[j].button)
                                   if(conf.bars.mouse[j].func)
                                        conf.bars.mouse[j].func(conf.bars.mouse[j].cmd);

     /* Selbar */
     if(conf.bars.selbar && ev->window == infobar[selscreen].selbar->win)
          for(i = 0; i < conf.selbar.nmouse; ++i)
               if(conf.selbar.mouse[i].tag == seltag[conf.selbar.mouse[i].screen]
                  || conf.selbar.mouse[i].tag < 0)
                    if(ev->button == conf.selbar.mouse[i].button)
                         if(conf.selbar.mouse[i].func)
                              conf.selbar.mouse[i].func(conf.selbar.mouse[i].cmd);

     /* Tags */
     for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
          if(ev->window == infobar[selscreen].tags[i]->win)
          {
               for(j = 0; j < tags[selscreen][i].nmouse; ++j)
                    if(ev->button == tags[selscreen][i].mouse[j].button)
                         if(tags[selscreen][i].mouse[j].func)
                              tags[selscreen][i].mouse[j].func(tags[selscreen][i].mouse[j].cmd);

               /* Mouse button action on tag */
               if(ev->button == conf.mouse_tag_action[TagSel])
                    tag_set(i);
               else if(ev->button == conf.mouse_tag_action[TagTransfert])
                    tag_transfert(sel, i);
               else if(ev->button == conf.mouse_tag_action[TagAdd])
                    tag_additional(selscreen, seltag[selscreen], i);
               else if(ev->button == conf.mouse_tag_action[TagNext])
                    tag_set(seltag[selscreen] + 1);
               else if(ev->button == conf.mouse_tag_action[TagPrev])
                    tag_set(seltag[selscreen] - 1);
          }

     /* Layout button */
     if(ev->window == infobar[selscreen].layout_button->win && conf.nlayout > 1)
     {
          if(conf.layout_system && (ev->button == Button1 || ev->button == Button3)) /* True -> menu */
          {
               menulayout.y = spgeo[selscreen].y + infobar[selscreen].layout_button->geo.y + INFOBARH;
               menulayout.x = infobar[selscreen].layout_button->geo.x + (sgeo[selscreen].x - BORDH);

               if(infobar[selscreen].geo.y != spgeo[selscreen].y)
                    menulayout.y = infobar[selscreen].geo.y - (INFOBARH * menulayout.nitem) - SHADH;

               uicb_menu("menulayout");
          }
          else
          {
               switch(ev->button)
               {
               case Button1: case Button4: layoutswitch(True);  break;
               case Button3: case Button5: layoutswitch(False); break;
               }
          }
     }

     return;
}

/* ClientMessage handle event
 *\param ev XClientMessageEvent pointer
*/
static void
clientmessageevent(XClientMessageEvent *ev)
{
     Client *c;
     Systray *sy;
     int s, i, mess_t = 0;
     Atom rt;
     int rf;
     ulong ir, il;
     uchar *ret = NULL;
     uchar *ret_cmd = NULL;
     void (*func)(uicb_t);

     if(ev->format != 32)
          return;

     s = screen_count();

     for(i = 0; i < net_last + s; ++i)
          if(net_atom[i] == ev->message_type)
               mess_t = i;

     if(ev->window == ROOT)
     {
          /* Manage _NET_CURRENT_DESKTOP */
          if(mess_t == net_current_desktop
             && ev->data.l[0] >= 0
             && ev->data.l[0] < conf.ntag[selscreen])
               tag_set((int)(ev->data.l[0] + 1));

          /* Manage _WMFS_SET_SCREEN */
          if(mess_t == wmfs_set_screen
             && ev->data.l[0] >= 0
             && ev->data.l[0] <= s)
               screen_set_sel((int)(ev->data.l[0]));

          /* Manage _NET_ACTIVE_WINDOW */
          else if(mess_t == net_active_window)
          {
               if((c = client_gb_win(ev->window)))
                    client_focus(c);
               else if((sy = systray_find(ev->data.l[0])))
                    XSetInputFocus(dpy, sy->win, RevertToNone, CurrentTime);
          }
     }
     else if(ev->window == traywin)
     {
          /* Manage _NET_WM_SYSTEM_TRAY_OPCODE */
          if(mess_t == net_wm_system_tray_opcode)
          {
               if(ev->data.l[1] == XEMBED_EMBEDDED_NOTIFY)
               {
                    systray_add(ev->data.l[2]);
                    systray_update();
               }
               else if(ev->data.l[1] == XEMBED_REQUEST_FOCUS)
                    if((sy = systray_find(ev->data.l[2])))
                         ewmh_send_message(sy->win, sy->win, "_XEMBED",
                                   XEMBED_FOCUS_IN, XEMBED_FOCUS_CURRENT, 0, 0, 0);
          }
     }

     /* Manage _NET_WM_STATE */
     if(mess_t == net_wm_state)
          if((c = client_gb_win(ev->window)))
               ewmh_manage_net_wm_state(ev->data.l, c);

     /* Manage _NET_CLOSE_WINDOW */
     if(mess_t == net_close_window)
          if((c = client_gb_win(ev->window)))
               client_kill(c);

     /* Manage _NET_WM_DESKTOP */
     if(mess_t == net_wm_desktop)
          if((c = client_gb_win(ev->window)) && ev->data.l[0] != (long)0xFFFFFFFF)
               tag_transfert(c, ev->data.l[0]);

     /* Manage _WMFS_STATUSTEXT_x */
     if(mess_t >= wmfs_statustext && ev->data.l[4] == True)
     {
          if(XGetWindowProperty(dpy, ROOT, net_atom[mess_t], 0, 4096,
                                False, net_atom[utf8_string], &rt, &rf, &ir, &il, &ret) == Success)
          {
               statustext_handle(mess_t - wmfs_statustext, (char*)ret);
               XFree(ret);
          }
     }

     /* Manage _WMFS_FUNCTION && _WMFS_CMD */
     if((mess_t == wmfs_function && ev->data.l[4] == True)
        || (mess_t == wmfs_cmd && ev->data.l[4] == True))
     {
          XGetWindowProperty(dpy, ROOT, net_atom[wmfs_function], 0, 4096,
                    False, net_atom[utf8_string], &rt, &rf, &ir, &il, &ret);

          XGetWindowProperty(dpy, ROOT, net_atom[wmfs_cmd], 0, 4096,
                    False, net_atom[utf8_string], &rt, &rf, &ir, &il, &ret_cmd);

          if((func = name_to_func((char*)ret, func_list)))
               func((uicb_t)ret_cmd);

          XFree(ret_cmd);
          XFree(ret);
     }

     /* Manage _WMFS_UPDATE_HINTS */
     if(mess_t == wmfs_update_hints)
     {
          ewmh_get_number_of_desktop();
          ewmh_update_current_tag_prop();
          ewmh_get_client_list();
          ewmh_get_desktop_names();
          ewmh_set_desktop_geometry();
          screen_count();
          screen_get_sel();
     }

     if(mess_t == wmfs_update_status
               && estatus)
          spawn(conf.status_path);

     return;
}

/** ConfigureRequesthandle events
 * \param ev XConfigureRequestEvent pointer
*/
static void
configureevent(XConfigureRequestEvent *ev)
{
     XWindowChanges wc;
     Client *c;

     /* Check part */
     if((c = client_gb_win(ev->window))
        || (c = client_gb_win(ev->window)))
     {
          CHECK(!(c->flags & LMaxFlag));
          CHECK(!(c->flags & MaxFlag));
          CHECK(!(c->flags & FSSFlag));
     }

     if((c = client_gb_win(ev->window)))
     {
          if(ev->value_mask & CWX)
               c->geo.x = ev->x + BORDH;
          if(ev->value_mask & CWY)
               c->geo.y = ev->y + TBARH;
          if(ev->value_mask & CWWidth)
               c->geo.width = ev->width;
          if(ev->value_mask & CWHeight)
               c->geo.height = ev->height;

          if(c->flags & FreeFlag || !(c->flags & (TileFlag | LMaxFlag)))
               client_moveresize(c, c->geo, False);
          else
          {
               client_configure(c);
               arrange(c->screen, True);
          }
     }
     else
     {
          wc.x            = ev->x;
          wc.y            = ev->y;
          wc.width        = ev->width;
          wc.height       = ev->height;
          wc.border_width = ev->border_width;
          wc.sibling      = ev->above;
          wc.stack_mode   = ev->detail;

          XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
     }

     return;
}

/** DestroyNotify handle event
 * \param ev XDestroyWindowEvent pointer
*/
static void
destroynotify(XDestroyWindowEvent *ev)
{
     Client *c;
     Systray *s;

     if((c = client_gb_win(ev->window)))
     {
          client_unmanage(c);
          XSetErrorHandler(errorhandler);
     }
     else if((s = systray_find(ev->window)))
     {
          setwinstate(s->win, WithdrawnState);
          systray_del(s);
          systray_update();
     }

     return;
}

/** EnterNotify handle event
 * \param ev XCrossingEvent pointer
*/
static void
enternotify(XCrossingEvent *ev)
{
     Client *c;
     int n;

     if((ev->mode != NotifyNormal
         || ev->detail == NotifyInferior)
        && ev->window != ROOT)
          return;

     /* Don't handle EnterNotify event if it's about systray */
     if(systray_find(ev->window) || ev->window == traywin)
          return;

     if(conf.focus_fmouse)
     {
          if((c = client_gb_win(ev->window))
                    || (c = client_gb_frame(ev->window))
                    || (c = client_gb_titlebar(ev->window))
                    || (c = client_gb_button(ev->window, &n)))
               client_focus(c);
          else
               client_focus(NULL);
     }


     return;
}

/** ExposeEvent handle event
 * \param ev XExposeEvent pointer
*/
static void
expose(XExposeEvent *ev)
{
     Client *c;
     int i, sc;

     /* InfoBar member */
     for(sc = 0; sc < screen_count(); ++sc)
     {
          if(ev->window == infobar[sc].bar->win)
               barwin_refresh(infobar[sc].bar);
          if(ev->window == infobar[sc].layout_button->win)
               barwin_refresh(infobar[sc].layout_button);
          if(conf.bars.selbar && ev->window == infobar[sc].selbar->win)
               barwin_refresh(infobar[sc].selbar);
          for(i = 1; i < conf.ntag[sc] + 1; ++i)
               if(ev->window == infobar[sc].tags[i]->win)
                    barwin_refresh(infobar[sc].tags[i]);
     }

     /* Client frame */
     if((c = client_gb_titlebar(ev->window)))
          frame_update(c);

     return;
}

/** FocusChange handle event
 * \param ev XFocusChangeEvent pointer
 * \return
*/
static void
focusin(XFocusChangeEvent *ev)
{
     if(sel && ev->window != sel->win)
          client_focus(sel);

     return;
}

/** KeyPress handle event
 * \param ev XKeyPressedEvent pointer
*/
static void
keypress(XKeyPressedEvent *ev)
{
     int i;
     KeySym keysym;

     keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
     for(i = 0; i < conf.nkeybind; ++i)
          if(keysym == keys[i].keysym
             && (keys[i].mod & ~(numlockmask | LockMask))
             == (ev->state & ~(numlockmask | LockMask))
             && keys[i].func)
               keys[i].func(keys[i].cmd);

     return;
}

/** MappingNotify handle event
 * \param ev XMappingEvent pointer
*/
static void
mappingnotify(XMappingEvent *ev)
{
     XRefreshKeyboardMapping(ev);

     if(ev->request == MappingKeyboard)
          grabkeys();

     return;
}

/** MapNotify handle event
  * \param ev XMapEvent pointer
  */
static void
mapnotify(XMapEvent *ev)
{
     Client *c;
     Systray *s;

     if(ev->window != ev->event && !ev->send_event)
          return;

     if((c = client_gb_win(ev->window)))
          setwinstate(c->win, NormalState);
     else if((s = systray_find(ev->window)))
     {
          setwinstate(s->win, NormalState);
          ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, 0, 0);
     }

     return;
}

/** MapRequest handle event
 * \param ev XMapRequestEvent pointer
*/
static void
maprequest(XMapRequestEvent *ev)
{
     XWindowAttributes at;
     Systray *s;

     CHECK(XGetWindowAttributes(dpy, ev->window, &at));
     CHECK(!at.override_redirect);

     if((s = systray_find(ev->window)))
     {
          ewmh_send_message(s->win, s->win, "_XEMBED", CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, 0, 0);
          systray_update();
     }
     else if(!client_gb_win(ev->window))
          client_manage(ev->window, &at, True);

     return;
}

/** PropertyNotify handle event
 * \param ev XPropertyEvent pointer
*/
static void
propertynotify(XPropertyEvent *ev)
{
     Client *c;
     Systray *s;
     Window trans;
     XWMHints *h;

     if(ev->state == PropertyDelete)
          return;

     if((s = systray_find(ev->window)))
     {
          systray_state(s);
          systray_update();
     }

     if((c = client_gb_win(ev->window)))
     {
          switch(ev->atom)
          {
          case XA_WM_TRANSIENT_FOR:
               XGetTransientForHint(dpy, c->win, &trans);
               if((c->flags & TileFlag || c->flags & MaxFlag))
                    if(((c->flags & HintFlag && (client_gb_win(trans) != NULL)))
                              || (!(c->flags & HintFlag && (client_gb_win(trans) != NULL))))
                         arrange(c->screen, True);
               break;
          case XA_WM_NORMAL_HINTS:
               client_size_hints(c);
               break;
          case XA_WM_HINTS:
               if((h = XGetWMHints(dpy, c->win)) && (h->flags & XUrgencyHint) && c != sel)
               {
                    client_urgent(c, True);
                    XFree(h);
               }
               break;
          case XA_WM_NAME:
               client_get_name(c);
               break;
          default:
               if(ev->atom == net_atom[net_wm_name])
                    client_get_name(c);
               break;
          }
     }

     return;
}

/** XReparentEvent handle event
 * \param ev XReparentEvent pointer
 */
static void
reparentnotify(XReparentEvent *ev)
{
     (void)ev;

     return;
}

/** SelectionClearEvent handle event
 * \param ev XSelectionClearEvent pointer
 */
static void
selectionclearevent(XSelectionClearEvent *ev)
{
     /* Getting selection if lost it */
     if(ev->window == traywin)
          systray_acquire();

     systray_update();

     return;
}

/** UnmapNotify handle event
 * \param ev XUnmapEvent pointer
 */
static void
unmapnotify(XUnmapEvent *ev)
{
     Client *c;
     Systray *s;

     if((c = client_gb_win(ev->window))
        && ev->send_event
        && !(c->flags & HideFlag))
     {
          client_unmanage(c);
          XSetErrorHandler(errorhandler);
     }

     if((s = systray_find(ev->window)))
     {
          systray_del(s);
          systray_update();
     }

     return;
}

/** Key grabbing function
*/
void
grabkeys(void)
{
     int i;
     KeyCode code;

     XUngrabKey(dpy, AnyKey, AnyModifier, ROOT);
     for(i = 0; i < conf.nkeybind; ++i)
          if((code = XKeysymToKeycode(dpy, keys[i].keysym)))
          {
               XGrabKey(dpy, code, keys[i].mod, ROOT, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(dpy, code, keys[i].mod | LockMask, ROOT, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(dpy, code, keys[i].mod | numlockmask, ROOT, True, GrabModeAsync, GrabModeAsync);
               XGrabKey(dpy, code, keys[i].mod | LockMask | numlockmask, ROOT, True, GrabModeAsync, GrabModeAsync);
          }

     return;
}

/** Event handle function: execute every function
 * handle by event
 * \param ev Event
 */
void
getevent(XEvent ev)
{

     switch(ev.type)
     {
     case ButtonPress:      buttonpress(&ev.xbutton);                 break;
     case ClientMessage:    clientmessageevent(&ev.xclient);          break;
     case ConfigureRequest: configureevent(&ev.xconfigurerequest);    break;
     case DestroyNotify:    destroynotify(&ev.xdestroywindow);        break;
     case EnterNotify:      enternotify(&ev.xcrossing);               break;
     case Expose:           expose(&ev.xexpose);                      break;
     case FocusIn:          focusin(&ev.xfocus);                      break;
     case KeyPress:         keypress(&ev.xkey);                       break;
     case MapNotify:        mapnotify(&ev.xmap);                      break;
     case MapRequest:       maprequest(&ev.xmaprequest);              break;
     case MappingNotify:    mappingnotify(&ev.xmapping);              break;
     case PropertyNotify:   propertynotify(&ev.xproperty);            break;
     case ReparentNotify:   reparentnotify(&ev.xreparent);            break;
     case SelectionClear:   selectionclearevent(&ev.xselectionclear); break;
     case UnmapNotify:      unmapnotify(&ev.xunmap);                  break;
     default:

#ifdef HAVE_XRANDR
          /* Check Xrandr event */
          if(ev.type == xrandr_event)
          {
               /* Update xrandr configuration */
               XRRUpdateConfiguration(&ev);

               /* Reload WMFS to update the screen(s) geometry changement */
               quit();
               for(; argv_global[0] && argv_global[0] == ' '; ++argv_global);
               execvp(argv_global, all_argv);
          }
#endif /* HAVE_XRANDR */
          break;
     }

     return;
}
