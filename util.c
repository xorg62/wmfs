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

void*
emalloc(uint element, uint size)
{
     void *ret = calloc(element, size);

     if(!ret)
          fprintf(stderr,"WMFS Error: could not malloc() %u bytes\n", size);

     return ret;
}

ulong
getcolor(char *color)
{
     XColor xcolor;

     if(!XAllocNamedColor(dpy,DefaultColormap(dpy, screen), color, &xcolor, &xcolor))
          fprintf(stderr,"WMFS Error: cannot allocate color \"%s\"\n", color);
     return xcolor.pixel;
}

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

ushort
textw(const char *text)
{
     XGlyphInfo gl;

     XftTextExtentsUtf8(dpy, xftfont, (FcChar8 *)text, strlen(text), &gl);

     return gl.width + xftfont->descent;
}

void
xprint(Drawable d, int x, int y, char* fg, uint bg, int decx, int decw, char *str)
{
     XftColor xftcolor;
     XftDraw *xftd;

     /* Transform X Drawable -> Xft Drawable */
     xftd = XftDrawCreate(dpy, d, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));

     /* Color the text font */
     XSetForeground(dpy, gc, bg);
     XFillRectangle(dpy, d, gc, x - decx, 0, textw(str) - decw, barheight);

     /* Alloc text color and draw */
     XftColorAllocName(dpy,
                       DefaultVisual(dpy, screen),
                       DefaultColormap(dpy, screen),
                       fg, &xftcolor);
     XftDrawStringUtf8(xftd, &xftcolor, xftfont, x, y, (FcChar8 *)str, strlen(str));
     XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), &xftcolor);

     return;
}
