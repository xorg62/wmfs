/*
*      structs.h
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

#ifndef STRUCTS_H
#define STRUCTS_H

#include "wmfs.h"

#define NBUTTON      5
#define MAXTAG       36
#define MAXLAYOUT    3

/* Typedef */
typedef const char* uicb_t;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

/* Enum */
enum { CurNormal, CurResize, CurMove, CurLast };
enum { WMState, WMProtocols, WMName, WMDelete, WMLast };
enum { NetSupported, NetWMName, NetLast };

/* BarWindow Structure
 * (titlebar, topbar..) */
typedef struct
{
     Window win;
     Drawable dr;
     int x, y;
     uint w ,h;
     uint color;
     int bord;
} BarWindow;

/* Client Structure. */
typedef struct Client Client;
struct Client
{
     /* Client title */
     char *title;
     /* Tag num */
     int tag;
     /* Window attribute */
     int x, y, w, h;
     /* Old window attribute */
     int ox, oy, ow, oh;
     /* For resizehint usage */
     int basew, baseh, incw, inch;
     int maxw, maxh, minw, minh;
     int minax, maxax, minay, maxay;
     /* Window */
     Window win;
     /* Titlebar */
     BarWindow *tbar;
     /* Client Layout Information */
     Bool max, tile, free;
     Bool hint, hide, lmax;
     /* Struct in chains */
     Client *next;
     Client *prev;
};

/* Keybind Structure */
typedef struct
{
     uint mod;
     KeySym keysym;
     void (*func)(uicb_t);
     char *cmd;
} Key;


/* Bar Button */
typedef struct
{
     char *content;
     BarWindow *bw;
     char *fg_color;
     int bg_color;
     uint x;
     int nmousesec;
     void (*func[NBUTTON])(uicb_t);
     char *cmd[NBUTTON];
     uint mouse[NBUTTON];
} BarButton;

/* Layout Structure */
typedef struct
{
     char *symbol;
     void (*func)(void);
} Layout;


/* Tag Structure */
typedef struct
{
     char *name;
     float mwfact;
     int nmaster;
     Bool resizehint;
     Layout layout;
} Tag;

/* Configuration structure */
typedef struct
{
     char *font;
     bool raisefocus;
     bool raiseswitch;
     bool bartop;
     int borderheight;
     int ttbarheight;
     int tagbordwidth;
     struct
     {
          /* Only the colors will be use for text
           * are 'char*' (for xprint -> XftColorAllocName) */
          uint bordernormal;
          uint borderfocus;
          uint bar;
          char *text;
          char *tagselfg;
          uint tagselbg;
          uint tagbord;
          char *layout_fg;
          uint layout_bg;
          char *ttbar_text_focus;
          char *ttbar_text_normal;
     } colors;
     Tag tag[MAXTAG];
     Layout layout[MAXLAYOUT];
     BarButton *barbutton;
     int ntag;
     int nkeybind;
     int nbutton;
     int nlayout;
} Conf;

/* Config.c struct */
typedef struct
{
     char *name;
     void *func;
} func_name_list_t;

typedef struct
{
      char *name;
      KeySym keysym;
} key_name_list_t;

typedef struct
{
     char *name;
     uint button;
} name_to_uint_t;

typedef struct
{
     char *name;
     char *content;
} Variable;

#endif /* STRUCTS_H */
