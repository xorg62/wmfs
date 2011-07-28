/*
*      util.c
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

#define _GNU_SOURCE
#include "wmfs.h"

/** malloc with error support and size_t overflow protection
 * \param nmemb number of objects
 * \param size size of single object
 * \return non null void pointer
 */
void *
xmalloc(size_t nmemb, size_t size)
{
     void *ret;

     if (SIZE_MAX / nmemb < size)
          err(EXIT_FAILURE, "xmalloc(%zu, %zu), "
                    "size_t overflow detected", nmemb, size);

     if ((ret = malloc(nmemb * size)) == NULL)
          err(EXIT_FAILURE, "malloc(%zu)", nmemb * size);

     return ret;
}

/** calloc with error support
 * \param nmemb Number of objects
 * \param size size of single object
 * \return non null void pointer
*/
void *
xcalloc(size_t nmemb, size_t size)
{
     void *ret;

     if ((ret = calloc(nmemb, size)) == NULL)
          err(EXIT_FAILURE, "calloc(%zu * %zu)", nmemb, size);

     return ret;
}

/** realloc with error support and size_t overflow check
 * \param ptr old pointer
 * \param nmemb number of objects
 * \param size size of single object
 * \return non null void pointer
 */
void *
xrealloc(void *ptr, size_t nmemb, size_t size)
{
     void *ret;

     if (SIZE_MAX / nmemb < size)
          err(EXIT_FAILURE, "xrealloc(%p, %zu, %zu), "
                    "size_t overflow detected", ptr, nmemb, size);

     if ((ret = realloc(ptr, nmemb * size)) == NULL)
          err(EXIT_FAILURE, "realloc(%p, %zu)", ptr, nmemb * size);

     return ret;
}

/** strdup with error support
 * \param str char pointer
 * \retun non null void pointer
 */
char *
xstrdup(const char *str)
{
     char *ret;

     if (str == NULL || (ret = strdup(str)) == NULL)
          err(EXIT_FAILURE, "strdup(%s)", str);

     return ret;
}

/** asprintf wrapper
 * \param strp target string
 * \param fmt format
 * \return non zero integer
 */
int
xasprintf(char **strp, const char *fmt, ...)
{
     int ret;
     va_list args;

     va_start(args, fmt);
     ret = vasprintf(strp, fmt, args);
     va_end(args);

     if (ret == -1)
          err(EXIT_FAILURE, "asprintf(%s)", fmt);

     return ret;
}


/** Get a color with a string
 * \param color Color string
 * \return Color pixel
*/
long
getcolor(char *color)
{
     XColor xcolor;

     if(!XAllocNamedColor(dpy, DefaultColormap(dpy, SCREEN), color, &xcolor, &xcolor))
          warnx("Error: cannot allocate color \"%s\".", color);

     return xcolor.pixel;
}

/** Set the window WM State
 * \param win Window target
 * \param state WM State
*/
void
setwinstate(Window win, long state)
{
     long data[] = {state, None};

     XChangeProperty(dpy, win, ATOM("WM_STATE"), ATOM("WM_STATE"), 32,
                     PropModeReplace, (uchar *)data, 2);

     return;
}

/* The following function are for configuration
   usage. {{{
*/
void*
name_to_func(char *name, const func_name_list_t *l)
{
     int i;

     if(name)
          for(i = 0; l[i].name; ++i)
               if(!strcmp(name, l[i].name))
                    return l[i].func;

     return NULL;
}

ulong
char_to_modkey(char *name, key_name_list_t key_l[])
{
     int i;

     if(name)
          for(i = 0; key_l[i].name; ++i)
               if(!strcmp(name, key_l[i].name))
                    return key_l[i].keysym;

     return NoSymbol;
}

uint
char_to_button(char *name, name_to_uint_t blist[])
{
     int i;

     if(name)
          for(i = 0; blist[i].name; ++i)
               if(!strcmp(name, blist[i].name))
                    return blist[i].button;

     return 0;
}

Layout
layout_name_to_struct(Layout lt[], char *name, int n, const func_name_list_t llist[])
{
     int i;

     for(i = 0; i < n; ++i)
          if(lt[i].func == name_to_func(name, llist))
               return lt[i];

     return lt[0];
}
/* }}} */

/** Execute a sh command
 * \param cmd Command
 * \return child pid
*/
pid_t
spawn(const char *format, ...)
{
     char *sh = NULL;
     char cmd[512];
     va_list ap;
     pid_t pid;
     size_t len;

     va_start(ap, format);
     len = vsnprintf(cmd, sizeof(cmd), format, ap);
     va_end(ap);

     if (len >= sizeof(cmd))
     {
          warnx("command too long (> 512 bytes)");
          return -1;
     }

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";

     if((pid = fork()) == 0)
     {
          if(dpy)
               close(ConnectionNumber(dpy));
          setsid();
          if (execl(sh, sh, "-c", cmd, (char*)NULL) == -1)
               warn("execl(sh -c %s)", cmd);
          exit(EXIT_FAILURE);
     }
     else if (pid == -1)
          warn("fork");

     return pid;
}

/** Swap two pointer.
 *\param x First pointer
 *\param y Second pointer
*/
void
swap_ptr(void **x, void **y)
{
     void *t = *x;

     *x = *y;
     *y = t;

     return;
}

/** Execute a sh command
 * \param cmd Command (uicb_t type)
*/
void
uicb_spawn(uicb_t cmd)
{
     spawn("%s", cmd);

     return;
}

/* To use ~/ shortcut.. */
char*
patht(char *path)
{
     static char ret[512];

     if(!path)
          return NULL;

     strncpy(ret, path, sizeof(ret));
     ret[sizeof(ret) - 1] = 0;
     if(strstr(path, "~/"))
          snprintf(ret, sizeof(ret), "%s/%s", getenv("HOME"), path + 2);

     return ret;
}

int
qsort_string_compare (const void * a, const void * b)
{
     return (strcmp(*(char **)a, *(char **)b));
}

