/*
*      util.c
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
          fprintf(stderr,"WMFS Error: could not malloc() %u bytes\n", size);

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
          fprintf(stderr,"WMFS Error: cannot allocate color \"%s\"\n", color);

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
 * Thanks linkkd.
 * \param str char pointer
*/
char*
_strdup(char const *str)
{
     char *ret = emalloc(strlen(str) + 1, sizeof(char));

     memset(ret, strlen(str) + 1, 0);
     strcpy(ret, str);

     return ret;
}

/* The following function are for configuration
   usage. {{{
*/
void*
name_to_func(char *name, func_name_list_t *l)
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
layout_name_to_struct(Layout lt[], char *name, int n, func_name_list_t llist[])
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
*/
void
spawn(const char *format, ...)
{
     char *sh = NULL;
     char cmd[512];
     va_list ap;

     va_start(ap, format);
     vsprintf(cmd, format, ap);
     va_end(ap);

     if(!strlen(cmd))
          return;

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";

     if(fork() == 0)
     {
          if(fork() == 0)
          {
               if(dpy)
                    close(ConnectionNumber(dpy));
               setsid();
               execl(sh, sh, "-c", cmd, (char*)NULL);
               exit(EXIT_SUCCESS);
          }
          exit(EXIT_SUCCESS);
     }

     return;
}

/** Execute a sh command
 * \param cmd Command (uicb_t type)
*/
void
uicb_spawn(uicb_t cmd)
{
     spawn(cmd);
}
