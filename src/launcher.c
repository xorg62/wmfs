/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <string.h>
#include <fts.h>
#include <sys/stat.h>
#include <X11/Xutil.h>

#include "wmfs.h"
#include "event.h"
#include "util.h"
#include "infobar.h"
#include "config.h"

static int
fts_alphasort(const FTSENT **a, const FTSENT **b)
{
     return (strcmp((*a)->fts_name, (*b)->fts_name));
}

static char **
complete_on_command(char *start)
{
     char **paths, *path, *p, **namelist = NULL;
     int count;
     FTS *tree;
     FTSENT *node;

     if(!(path = getenv("PATH")) || !start)
          return NULL;

     /* split PATH into paths */
     path = p = xstrdup(path);

     for(count = 1, p = path; strchr(p, ':'); ++p, ++count);

     paths = xcalloc(count, sizeof(*paths));

     for(paths[0] = p = path, count = 1; (p = strchr(p, ':')); ++p, ++count)
     {
          paths[count] = p + 1;
          *p = '\0';
     }

     paths[count] = NULL;

     if(!(tree = fts_open(paths, FTS_NOCHDIR, fts_alphasort)))
     {
          warn("fts_open");
          free(paths);
          free(path);
          return NULL;
     }

     count = 0;
     while((node = fts_read(tree)))
     {
          if(node->fts_level > 0)
               fts_set(tree, node, FTS_SKIP);

          if(node->fts_level
             && (node->fts_info & FTS_F)
             && (node->fts_info & FTS_NS)
             && (node->fts_statp->st_mode & S_IXOTH)
             && !strncmp(node->fts_name, start, strlen(start)))
          {
               namelist = xrealloc(namelist, ++count, sizeof(*namelist));
               namelist[count - 1] = xstrdup(node->fts_name + strlen(start));
          }
     }

     if(count)
     {
          namelist = xrealloc(namelist, ++count, sizeof(*namelist));
          namelist[count - 1] = NULL;
     }

     if(fts_close(tree))
          warn("fts_close");

     free(paths);
     free(path);

     return namelist;
}

/*
 * Complete a filename or directory name.
 * works like complete_on_command.
 */
static char **
complete_on_files(char *start)
{
     char *p, *home, *path, *dirname = NULL, *paths[2], **namelist = NULL;
     int count;
     FTS *tree;
     FTSENT *node;

     p = start;

     /*
      * Search the directory to open and set
      * the beginning of file to complete on pointer 'p'.
      */
     if(*p == '\0' || !strrchr(p, '/'))
          path = xstrdup(".");
     else
     {
          if(!(home = getenv("HOME")))
               return NULL;

          /* remplace ~ by $HOME in dirname */
          if(!strncmp(p, "~/", 2) && home)
               xasprintf(&dirname, "%s%s", home, p+1);
          else
               dirname = xstrdup(p);

          /* Set p to filename to be complete
           * and path the directory containing the file
           * /foooooo/baaaaaar/somethinglikethis<tab>
           * <---- path - ---><------- p ------>
           */
          p = strrchr(dirname, '/');
          if(p != dirname)
          {
               *(p++) = '\0';
               path = xstrdup(dirname);
          }
          else
          {
               path = xstrdup("/");
               p++;
          }
     }

     paths[0] = path;
     paths[1] = NULL;

     if(!(tree = fts_open(paths, FTS_NOCHDIR, fts_alphasort)))
     {
          warn("fts_open");
          free(dirname);
          free(path);
          return NULL;
     }

     count = 0;
     while((node = fts_read(tree)))
     {
          if(node->fts_level > 0)
               fts_set(tree, node, FTS_SKIP);

          if(node->fts_level && !strncmp(node->fts_name, p, strlen(p)))
          {
               namelist = xrealloc(namelist, ++count, sizeof(*namelist));
               namelist[count - 1] = xstrdup(node->fts_name + strlen(p));
          }
     }

     if(count)
     {
          namelist = xrealloc(namelist, ++count, sizeof(*namelist));
          namelist[count - 1] = NULL;
     }

     if(fts_close(tree))
          warn("fts_close");

     free(dirname);
     free(path);

     return namelist;
}

static void
complete_cache_free(struct launcher_ccache *cache)
{
     int i;

     /* release memory */
     free(cache->start);

     if(cache->namelist)
     {
          for(i = 0; cache->namelist[i]; i++)
               free(cache->namelist[i]);
          free(cache->namelist);
     }

     /* init */
     cache->hits = 0;
     cache->start = NULL;
     cache->namelist = NULL;
}

static char *
complete(struct launcher_ccache *cache, char *start)
{
     char *p = NULL, *comp = NULL;

     if(!start || !cache)
          return NULL;

     if((p = strrchr(start, ' ')))
          p++;
     else
          p = start;

     if(cache->start && !strcmp(cache->start, start))
     {
          if(cache->namelist && !cache->namelist[cache->hits])
               cache->hits = 0;
     }
     else
     {
          complete_cache_free(cache);
          cache->start = xstrdup(start);

          if(p == start)
               cache->namelist = complete_on_command(p);
          else
               cache->namelist = complete_on_files(p);

     }

     if(cache->namelist && cache->namelist[cache->hits])
          comp = cache->namelist[cache->hits];

     return comp;
}

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
     struct launcher_ccache cache = {NULL, NULL, 0};
     bool loop = true, found = false, lastwastab = false;
     char tmpbuf[512] = { 0 }, buf[512] = { 0 };
     char tmp[32] = { 0 };
     char *p, *data, *arg, *end, *cmd = xstrdup(l->command);
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

          /* Completion */
          case XK_Tab:
               buf[pos] = '\0';
               if(lastwastab)
                    ++cache.hits;
               else
               {
                    cache.hits = 0;
                    strncpy(tmpbuf, buf, sizeof(tmpbuf));
               }

               if(pos && (end = complete(&cache, tmpbuf)))
               {
                    strncpy(buf, tmpbuf, sizeof(buf));
                    strncat(buf, end, sizeof(buf));
                    found = true;
               }

               lastwastab = true;

               /* start a new round of tabbing */
               if(!found)
                    cache.hits = 0;

               pos = strlen(buf);
               break;

          case XK_BackSpace:
               lastwastab = false;
               if(pos)
                    buf[--pos] = '\0';
               break;

          default:
               lastwastab = false;
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

     complete_cache_free(&cache);
     free(cmd);
     free(data);

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
