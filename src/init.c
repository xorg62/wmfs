/*
*      wmfs.c
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

/** Init WMFS
*/
void
init(void)
{
     /* First init */
     init_gc();
     init_font();
     init_cursor();
     init_key();
     init_root();
     screen_init_geo();
     infobar_init();
     ewmh_get_current_desktop();
     grabkeys();

     /* Warning about font */
     if(TBARH + BORDH < font->height)
          fprintf(stderr, "WMFS Warning: Font too big, can't draw any text in the titlebar.\n");

     return;
}

/** Init the font
*/
void
init_font(void)
{
     font = XftFontOpenName(dpy, SCREEN, conf.font);
     if(!font)
     {
          fprintf(stderr, "WMFS Error: Cannot initialize font\n");
          font = XftFontOpenName(dpy, SCREEN, "sans-10");
     }
}

/** Init the graphic context
*/
void
init_gc(void)
{
     XGCValues gcv;

     /* Bits sequences */
     const char pix_bits[] =
          {
               0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55,
               0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55,
               0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55,
               0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55
          };

     gc = DefaultGC(dpy, SCREEN);

     gcv.function   = GXcopy;
     gcv.fill_style = FillStippled;
     gcv.stipple    = XCreateBitmapFromData(dpy, ROOT, pix_bits, 10, 4);
     gc_stipple     = XCreateGC(dpy, ROOT, GCFunction|GCFillStyle|GCStipple, &gcv);

     return;
}

/** Init WMFS cursor
*/
void
init_cursor(void)
{
     cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
     cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
     cursor[CurMove]   = XCreateFontCursor(dpy, XC_fleur);

     return;
}

/** Init key modifier
*/
void
init_key(void)
{
     int i, j;
     XModifierKeymap *modmap = XGetModifierMapping(dpy);

     for(i = 0; i < 8; i++)
          for(j = 0; j < modmap->max_keypermod; ++j)
               if(modmap->modifiermap[i * modmap->max_keypermod + j]
                  == XKeysymToKeycode(dpy, XK_Num_Lock))
                    numlockmask = (1 << i);

     XFreeModifiermap(modmap);

     return;
}

/** Init root Window
*/
void
init_root(void)
{
     XSetWindowAttributes at;

     at.event_mask = KeyMask | ButtonMask | MouseMask
          | SubstructureRedirectMask | SubstructureNotifyMask |StructureNotifyMask;

     at.cursor = cursor[CurNormal];
     XChangeWindowAttributes(dpy, ROOT, CWEventMask | CWCursor, &at);
     if(conf.root.background_command)
          uicb_spawn(conf.root.background_command);

     ewmh_init_hints();
     ewmh_get_number_of_desktop();
     ewmh_get_desktop_names();

     return;
}


