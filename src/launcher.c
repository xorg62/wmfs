/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h>
#include <string.h>

#include "wmfs.h"
#include "event.h"
#include "util.h"
#include "infobar.h"
#include "config.h"

#define LAUNCHER_INIT_ELEM(width)                       \
     SLIST_FOREACH(ib, &W->screen->infobars, next)      \
     {                                                  \
          TAILQ_FOREACH(e, &ib->elements, next)         \
               if(e->type == ElemLauncher)              \
               {                                        \
                    e->geo.w = width;                   \
                    e->data = data;                     \
               }                                        \
          infobar_elem_reinit(ib);                      \
     }

static void
launcher_process(struct launcher *l)
{
     struct infobar *ib;
     struct element *e;
     bool loop = true;
     char buf[512] = { 0 };
     char tmp[32] = { 0 };
     char *p, *data, *arg, *cmd = xstrdup(l->command);
     int i, pos = 0, histpos = 0;
     void (*func)(Uicb);
     XEvent ev;
     KeySym ks;

     W->flags |= WMFS_LAUNCHER;

     /* Prepare elements */
     xasprintf(&data, "%s ", l->prompt);
     LAUNCHER_INIT_ELEM(l->width);

     XGrabKeyboard(W->dpy, W->root, true, GrabModeAsync, GrabModeAsync, CurrentTime);

     while(loop)
     {
          XNextEvent(W->dpy, &ev);

          if(ev.type != KeyPress)
          {
               EVENT_HANDLE(&ev);
               continue;
          }

          /* Get pressed key */
          XLookupString(&ev.xkey, tmp, sizeof(tmp), &ks, 0);

          /* Check Ctrl-c / Ctrl-d */
          if(ev.xkey.state & ControlMask)
          {
               switch(ks)
               {
               case XK_c:
               case XK_d:
                    ks = XK_Escape;
                    break;
               case XK_p:
                    ks = XK_Up;
                    break;
               case XK_n:
                    ks = XK_Down;
                    break;
               }
          }

          /* Check if there is a keypad */
          if(IsKeypadKey(ks) && ks == XK_KP_Enter)
               ks = XK_Return;

          /* Manage pressed keys */
          switch(ks)
          {
          case XK_Up:
               if(l->nhisto)
               {
                    if(histpos >= (int)l->nhisto)
                         histpos = 0;
                    strncpy(buf, l->histo[l->nhisto - ++histpos], sizeof(buf));
                    pos = strlen(buf);
               }
               break;

          case XK_Down:
               if(l->nhisto && histpos > 0 && histpos < (int)l->nhisto)
               {
                    strncpy(buf, l->histo[l->nhisto - --histpos], sizeof(buf));
                    pos = strlen(buf);
               }
               break;

          case XK_Return:
               /* Get function name only, if cmds are added in command */
               arg = NULL;
               if((p = strchr(cmd, ' ')))
               {
                    *p = '\0';
                    xasprintf(&arg, "%s %s", p + 1, buf);
               }

               if((func = uicb_name_func(cmd)))
               {
                    if(arg)
                    {
                         func(arg);
                         free(arg);
                    }
                    else
                         func(buf);
               }

               /* Histo */
               if(l->nhisto + 1 > HISTOLEN)
               {
                    for(i = l->nhisto - 1; i > 1; --i)
                         strncpy(l->histo[i], l->histo[i - 1], sizeof(l->histo[i]));

                    l->nhisto = 0;
               }
               /* Store in histo array */
               strncpy(l->histo[l->nhisto++], buf, sizeof(buf));

               loop = false;
               break;

          case XK_Escape:
               loop = false;
               break;

          /* TODO: Completion */
          case XK_Tab:
               break;

          case XK_BackSpace:
               if(pos)
                    buf[--pos] = '\0';
               break;

          default:
               strncat(buf, tmp, sizeof(tmp));
               ++pos;
               break;
          }

          free(data);
          xasprintf(&data, "%s %s", l->prompt, buf);

          /* Update EVERY launcher element of the screen */
          SLIST_FOREACH(ib, &W->screen->infobars, next)
          {
               TAILQ_FOREACH(e, &ib->elements, next)
               {
                    if(e->type != ElemLauncher)
                         continue;

                    e->data = data;
                    e->func_update(e);
               }
          }
     }

     XUngrabKeyboard(W->dpy, CurrentTime);

     free(cmd);

     /* 'Close' launcher elements */
     W->flags ^= WMFS_LAUNCHER;
     data = NULL;
     LAUNCHER_INIT_ELEM(1);
}

void
uicb_launcher(Uicb cmd)
{
     struct launcher *l;

     SLIST_FOREACH(l, &W->h.launcher, next)
          if(!strcmp(l->name, cmd))
          {
               launcher_process(l);
               break;
          }
}
