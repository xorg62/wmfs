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

/** Free with a conditions
 * \param ptr void pointer
*/
void
efree(void *ptr)
{
     if(ptr)
          free(ptr);

     return;
}

/** Get a color with a string
 * \param color Color string
 * \return Color pixel
*/
ulong
getcolor(char *color)
{
     XColor xcolor;

     if(!XAllocNamedColor(dpy,DefaultColormap(dpy, screen), color, &xcolor, &xcolor))
          fprintf(stderr,"WMFS Error: cannot allocate color \"%s\"\n", color);
     return xcolor.pixel;
}

/** Get a Window WM State
 * \param win Window
 * \return The state
*/
long
getwinstate(Window win)
{
     int f;
     long ret = -1;
     ulong n, e;
     uchar *p = NULL;
     Atom at;

     if(XGetWindowProperty(dpy, win, wm_atom[WMState], 0L, 2L, False, wm_atom[WMState],
                           &at, &f, &n, &e, (uchar **)&p) != Success)
          return -1;

     if(n != 0)
          ret = *p;
     free(p);

     return ret;
}

/** Round function
 * \param x double type
 * \return the round of x
*/
double
round(double x)
{
     return (x > 0) ? x + 0.5 : x - 0.5;
}

/** Set the window WM State
 * \param win Window target
 * \param state WM State
*/
void
setwinstate(Window win, long state)
{
     long data[] = {state, None};

     XChangeProperty(dpy, win, wm_atom[WMState], wm_atom[WMState], 32,
                     PropModeReplace, (unsigned char *)data, 2);

     return;
}

/** Execute a sh command
 * \param cmd Command
*/
void
uicb_spawn(uicb_t cmd)
{
     char *sh = NULL;

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";
     if(!strlen(cmd))
          return;
     if(fork() == 0)
     {
          if(fork() == 0)
          {
               if(dpy)
                    close(ConnectionNumber(dpy));
               setsid();
               execl(sh, sh, "-c", cmd, (char*)NULL);
               exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
     }

     return;
}
