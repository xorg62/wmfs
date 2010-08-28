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

#include "wmfs.h"

/** Calloc with an error message if there is a probleme
 * \param element Element
 * \param size Size
 * \return void pointer
*/
void*
emalloc(uint element, uint size)
{
     void *ret = calloc(element, size);

     if(!ret)
          warn("calloc()");

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

/** Enlight an hexadecimal color
 * \param col Color
 * \return The clarified color
*/
ulong
color_enlight(ulong col)
{
     if((col + 0x330000) < 0xffffff
        && (col + 0x003300) < 0xffffff
        && (col + 0x000033) < 0xffffff)
          return col + 0x333333;
     else
          return col;
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

/** My strdup. the strdup of string.h isn't ansi compatible..
 * Thanks linkdd.
 * \param str char pointer
*/
char*
_strdup(const char *str)
{
     char *ret = emalloc(strlen(str) + 1, sizeof(char));

     strcpy(ret, str);

     return ret;
}

/* The following function are for configuration
   usage. {{{
*/
void*
name_to_func(char *name, const func_name_list_t *l)
{
     int i;

     if(name)
          for(i = 0; l[i].name ; ++i)
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

char*
alias_to_str(char *conf_choice)
{
     int i;
     char *tmpchar = NULL;

     if(!conf_choice)
          return 0;

     if(conf.alias)
          for(i = 0; conf.alias[i].name; i++)
               if(!strcmp(conf_choice, conf.alias[i].name))
                    tmpchar = conf.alias[i].content;

     if(tmpchar)
          return _strdup(tmpchar);
     else
          return _strdup(conf_choice);

     return NULL;
}
/* }}} */

/** Get the mouse pointer position.
*/
XRectangle
get_mouse_pos(void)
{
     Window dum;
     int d, u;
     XRectangle ret;

     XQueryPointer(dpy, ROOT, &dum, &dum, (int*)&ret.x, (int*)&ret.y, &d, &d, (uint *)&u);

     return ret;
}

/** Execute a sh command
 * \param cmd Command
 * \return child pid
*/
int
spawn(const char *format, ...)
{
     char *sh = NULL;
     char cmd[512];
     va_list ap;
     pid_t pid, ret;
     int p[2], len;

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

     if (pipe(p) == -1)
     {
          warn("pipe");
          return -1;
     }

     if((pid = fork()) == 0)
     {
          close(p[0]);
          if((pid = fork()) == 0)
          {
               if(dpy)
                    close(ConnectionNumber(dpy));
               setsid();
               execl(sh, sh, "-c", cmd, (char*)NULL);
               exit(EXIT_FAILURE);
          }
          write(p[1], &pid, sizeof(pid_t));
          close(p[1]);
          exit(EXIT_SUCCESS);
     }
     else if (pid != -1)
     {
          close(p[1]);
          if (sizeof(pid_t) != read(p[0], &ret, sizeof(pid_t)))
          {
               warn("read");
               ret = -1;
          }
          close(p[0]);
          waitpid(pid, NULL, 0);
     }
     else
     {
          warn("fork");
          ret = -1;
     }

     return ret;
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

#ifdef HAVE_IMLIB
/** Check images blocks in str and return properties
  * --> \i[x;y;w;h;name]\
  *\param im ImageAttr pointer, image properties
  *\param str String
  *\return n Lenght of i
  */
int
parse_image_block(ImageAttr *im, char *str)
{
     char as;
     int n, i, j, k;

     for(i = j = n = 0; i < strlen(str); ++i, ++j)
          if(sscanf(&str[i], "\\i[%d;%d;%d;%d;%512[^]]]%c", &im[n].x, &im[n].y, &im[n].w, &im[n].h, im[n].name, &as) == 6
                    && as == '\\')
               for(++n, ++i, --j; str[i] != as || str[i - 1] != ']'; ++i);
          else if(j != i)
               str[j] = str[i];

     for(k = j; k < i; str[k++] = 0);

     return n;
}
#endif /* HAVE_IMLIB */

char*
clean_value(char *str)
{
     int i;
     char c, *p;

     if(!str || !(p = _strdup(str)))
          return NULL;

     /* Remove useless spaces */
     for(; *p == ' '; ++p);
     for(; *(p + strlen(p) - 1) == ' '; *(p + strlen(p) - 1) = '\0');

     /* For string delimiter (" or ') */
     if(((c = *p) == '"' || (c = *p) == '\'') && strchr(p + 1, c))
     {
          for(++p, i = 0; p[i] && p[i] != c; ++i);
          p[i] = '\0';
     }

     return p;
}

/* To use ~/ shortcut.. */
char*
patht(char *path)
{
     static char ret[512];

     if(!path)
          return NULL;

     strcpy(ret, path);

     if(strstr(path, "~/"))
          sprintf(ret, "%s/%s", getenv("HOME"), path + 2);

     return ret;
}

int
qsort_string_compare (const void * a, const void * b)
{
     return (strcmp(*(char **)a, *(char **)b));
}

