/*
*      wmfs.c
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


const func_name_list_t layout_list[] =
{
     {"tile",                    tile },
     {"tile_right",              tile },
     {"tile_left",               tile_left },
     {"tile_top",                tile_top },
     {"tile_bottom",             tile_bottom },
     {"tile_grid",               grid_horizontal },
     {"tile_grid_horizontal",    grid_horizontal },
     {"tile_grid_vertical",      grid_vertical },
     {"grid",                    grid_horizontal },
     {"mirror_vertical",         mirror_vertical },
     {"tile_mirror_vertical",    mirror_vertical },
     {"mirror_horizontal",       mirror_horizontal },
     {"tile_mirror_horizontal",  mirror_horizontal },
     {"max",                     maxlayout },
     {"maxlayout",               maxlayout },
     {"freelayout",              freelayout },
     {"free",                    freelayout },
     { NULL, NULL }
};

/** Init the font
*/
static void
init_font(void)
{
#ifdef HAVE_XFT
     if(conf.use_xft)
     {
          if(!(font.font = XftFontOpenName(dpy, SCREEN, conf.font)))
          {
               warnx("WMFS Error: Cannot initialize Xft font");
               font.font = XftFontOpenName(dpy, SCREEN, "sans-10");
          }

          font.de     = font.font->descent;
          font.as     = font.font->ascent;
          font.height = font.font->height;
     }
     else
#endif /* HAVE_XFT */
     {
          char **misschar, **names, *defstring;
          int d;
          XFontStruct **xfs = NULL;

          /* locale support */
          setlocale(LC_CTYPE, "");

          if(!conf.font)
               conf.font = xstrdup("fixed");

          /* Using Font Set */
          if(!(font.fontset = XCreateFontSet(dpy, conf.font, &misschar, &d, &defstring)))
          {
               warnx("Can't load font '%s'", conf.font);
               font.fontset = XCreateFontSet(dpy, "fixed", &misschar, &d, &defstring);
          }

          XExtentsOfFontSet(font.fontset);
          XFontsOfFontSet(font.fontset, &xfs, &names);

          font.as    = xfs[0]->max_bounds.ascent;
          font.de    = xfs[0]->max_bounds.descent;
          font.width = xfs[0]->max_bounds.width;

          font.height = font.as + font.de;

          if(misschar)
               XFreeStringList(misschar);
     }

     /* Set font in _WMFS_FONT for eventual status tools */
     XChangeProperty(dpy, ROOT, net_atom[wmfs_font], net_atom[utf8_string], 8,
                    PropModeReplace, (uchar*)conf.font, strlen(conf.font));

     return;
}

/** Init the graphic context
*/
static void
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

     /* Stipple GC */
     gcv.function   = GXcopy;
     gcv.fill_style = FillStippled;
     gcv.stipple    = XCreateBitmapFromData(dpy, ROOT, pix_bits, 10, 4);
     gc_stipple     = XCreateGC(dpy, ROOT, GCFunction | GCFillStyle | GCStipple, &gcv);

     return;
}

/** Init WMFS cursor
*/
static void
init_cursor(void)
{
     cursor[CurNormal]      = XCreateFontCursor(dpy, XC_left_ptr);
     cursor[CurResize]      = XCreateFontCursor(dpy, XC_sizing);
     cursor[CurRightResize] = XCreateFontCursor(dpy, XC_lr_angle);
     cursor[CurLeftResize]  = XCreateFontCursor(dpy, XC_ll_angle);
     cursor[CurMove]        = XCreateFontCursor(dpy, XC_fleur);

     return;
}

/** Init key modifier
*/
static void
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
static void
init_root(void)
{
     XSetWindowAttributes at;

     at.event_mask = KeyMask | ButtonMask | MouseMask | PropertyChangeMask
          | SubstructureRedirectMask | SubstructureNotifyMask | StructureNotifyMask;

     at.cursor = cursor[CurNormal];
     XChangeWindowAttributes(dpy, ROOT, CWEventMask | CWCursor, &at);

     if(conf.root.background_command)
          spawn("%s", conf.root.background_command);

     ewmh_init_hints();
     ewmh_get_number_of_desktop();
     ewmh_get_desktop_names();

     return;
}

/** Init statustext shell script
  */
static void
init_status(void)
{
     struct stat st;
     char *home;

     conf.status_pid = -1;
     estatus = False;

     if(!conf.status_path)
     {
          if(!(home = getenv("HOME")))
          {
               warnx("HOME not set, can't launch status.sh");
               return;
          }

          conf.status_path = zmalloc(strlen(home) + strlen(DEF_STATUS) + 2);
          sprintf(conf.status_path, "%s/"DEF_STATUS, home);
     }

     if (stat(patht(conf.status_path), &st) == -1)
     {
          warn("%s", patht(conf.status_path));
          return;
     }

     if(st.st_size && st.st_mode & S_IXUSR)
          estatus = True;
     else
          warnx("status file specified in configuratin (status_path) or present in wmfs directory can't be executed, try 'chmod +x %s'.", patht(conf.status_path));

     return;
}

/** Init WMFS
*/
void
init(void)
{
     /* First init */
     ewmh_init_hints();
     init_conf();
     init_gc();
     init_font();
     init_cursor();
     init_key();
     init_root();
     screen_init_geo();
     event_make_array();
     infobar_init();
     systray_acquire();
     init_status();
     ewmh_update_current_tag_prop();
     grabkeys();

     return;
}

