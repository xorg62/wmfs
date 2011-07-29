/*
*      structs.h
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

#ifndef STRUCTS_H
#define STRUCTS_H

#include "wmfs.h"

#define NBUTTON        8
#define MAXTAG         36
#define NUM_OF_LAYOUT  11
#define HISTOLEN       128

/* Clients flags definition */
#define FreeFlag   0x01
#define MaxFlag    0x02
#define TileFlag   0x04
#define HideFlag   0x08
#define LMaxFlag   0x10
#define UnmapFlag  0x20
#define HintFlag   0x40
#define FSSFlag    0x80
#define AboveFlag  0x100
#define UrgentFlag 0x200
#define FLayFlag   0x400
#define DockFlag   0x800

/* Layout flags definition */
#define IntegrationFlag 0x01
#define ArrangeFlag     0x02

/* Tag flags definition */
#define TagUrgentFlag     0x01
#define ResizeHintFlag    0x02
#define RequestUpdateFlag 0x04
#define AboveFCFlag       0x08
#define CleanFactFlag     0x10
#define StayLastFlag      0x20
#define SplitFlag         0x40
#define FirstArrangeFlag  0x80
#define IgnoreEnterFlag   0x100

#define TagFlag(t) (1 << (t))

/* BarWindow flags definition */
#define MappedFlag   0x01
#define StippleFlag  0x02
#define BordFlag     0x04

/* XEMBED messages */
#define XEMBED_MAPPED                 (1 << 0)
#define XEMBED_EMBEDDED_NOTIFY        0
#define XEMBED_WINDOW_ACTIVATE        1
#define XEMBED_WINDOW_DEACTIVATE      2
#define XEMBED_REQUEST_FOCUS          3
#define XEMBED_FOCUS_IN               4
#define XEMBED_FOCUS_OUT              5
#define XEMBED_FOCUS_NEXT             6
#define XEMBED_FOCUS_PREV             7
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON            10
#define XEMBED_MODALITY_OFF           11
#define XEMBED_REGISTER_ACCELERATOR   12
#define XEMBED_UNREGISTER_ACCELERATOR 13
#define XEMBED_ACTIVATE_ACCELERATOR   14

/* Details for  XEMBED_FOCUS_IN: */
#define XEMBED_FOCUS_CURRENT		0
#define XEMBED_FOCUS_FIRST 		1
#define XEMBED_FOCUS_LAST		2

/* Typedef */
typedef const char*    uicb_t;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef unsigned short ushort;
typedef unsigned char  uchar;

/* Enum */
enum { CurNormal, CurResize, CurRightResize, CurLeftResize, CurMove, CurLast };
enum { TagSel, TagTransfert, TagAdd, TagNext, TagPrev, TagActionLast };

/* Menu align */
enum { MA_Center = 0, MA_Left = 1, MA_Right = 2 };

/* Infobar position */
enum { IB_Hide = 0, IB_Bottom = 1, IB_Top = 2 };

typedef enum { Right = 0, Left, Top, Bottom, Center, PositionLast } Position;

/* Ewmh hints list */
enum
{
     net_supported,
     net_wm_name,
     net_client_list,
     net_frame_extents,
     net_number_of_desktops,
     net_current_desktop,
     net_desktop_names,
     net_desktop_geometry,
     net_active_window,
     net_close_window,
     net_wm_icon_name,
     net_wm_window_type,
     net_wm_pid,
     net_showing_desktop,
     net_supporting_wm_check,
     net_wm_window_opacity,
     net_wm_window_type_normal,
     net_wm_window_type_dock,
     net_wm_window_type_splash,
     net_wm_window_type_dialog,
     net_wm_desktop,
     net_wm_icon,
     net_wm_state,
     net_wm_state_fullscreen,
     net_wm_state_sticky,
     net_wm_state_demands_attention,
     net_wm_system_tray_opcode,
     net_system_tray_message_data,
     net_system_tray_s,
     net_system_tray_visual,
     net_system_tray_orientation,
     xembed,
     xembedinfo,
     manager,
     utf8_string,
     /* WMFS HINTS */
     wmfs_running,
     wmfs_update_hints,
     wmfs_current_tag,
     wmfs_current_screen,
     wmfs_current_layout,
     wmfs_tag_list,
     wmfs_mwfact,
     wmfs_nmaster,
     wmfs_set_screen,
     wmfs_screen_count,
     wmfs_function,
     wmfs_cmd,
     wmfs_font,
     wmfs_statustext,
     net_last
};

/* Geometry structure */
typedef struct
{
     int x, y;
     int width, height;
} Geo;

/*
 *  BarWindow Structure
 * (titlebar, infobar..)
 */
typedef struct
{
     Window win;
     Drawable dr;
     struct
     {
          /* Border Window */
          Window left, right, top, bottom;
          /* Border color */
          uint dark, light;

     } border;
     uint bg;
     char *fg;
     uint stipple_color;
     Geo geo;
     uint flags;
} BarWindow;

/* Client Structure. */
typedef struct Client Client;
struct Client
{
     /* Client title */
     char *title;
     /* Tag num */
     uint tag;
     int focusontag;
     /* Screen */
     int screen;
     /* Window attribute */
     Geo geo, pgeo;            /* Window geo, tiling pure geo */
     Geo tmp_geo, wrgeo;       /* Temporary geo, without resizehint geo */
     Geo frame_geo;            /* Frame geo */
     Geo ogeo;                 /* Old window attribute */
     Geo split_geo, free_geo;  /* Split & Free window attribute */
     /* Tile size factors */
     int tilefact[4];
     /* For resizehint usage */
     int basew, baseh, incw, inch;
     int maxw, maxh, minw, minh;
     int minax, maxax, minay, maxay;
     /* Client composant {{{ */
     Window win;
     Window *button;
     int button_last_x;
     BarWindow *titlebar;
     Window frame, resize[2];
     /* Border */
     Window right, left, top, bottom;
     /* }}} */
     struct
     {
          uint frame;
          char *fg;
          uint resizecorner;
          uint bordlight, borddark;
     } colors;
     /* Client Information by flags */
     uint flags;
     /* Simply-linked list */
     SLIST_ENTRY(Client) next;
};

/* Keybind Structure */
typedef struct
{
     uint mod;
     KeySym keysym;
     void (*func)(uicb_t);
     uicb_t cmd;
} Key;

/* Mouse Binding Struct */
typedef struct
{
     int tag;
     int screen;
     uint button;
     void (*func)(uicb_t);
     uicb_t cmd;
} MouseBinding;

/* InfoBar Struct */
typedef struct
{
     BarWindow *bar, *selbar;
     BarWindow *layout_button;
     BarWindow *tags_board, *tags[MAXTAG];
     Geo geo;
     int screen, position;
     char *statustext;
     bool need_update;
} InfoBar;

/* Layout Structure */
typedef struct
{
     uint flags;       /* Flags */
     Client *nc;       /* New client needing integration */
     Client ghost;     /* Ghost client to arrange hole in split */
     char *symbol;
     char *type;
     void (*func)(int screen);
} Layout;

/* Systray Structure */
typedef struct Systray Systray;
struct Systray
{
     Window win;
     Geo geo;
     SLIST_ENTRY(Systray) next;
};

/* Tag Structure */
typedef struct
{
     char *name;
     char **clients;
     int nclients;
     float mwfact;
     int nmaster;
     uint flags;
     int barpos;
     int prev_barpos;
     Layout layout;
     uint tagad;
     MouseBinding *mouse;
     int nmouse;
} Tag;

/* Menu Item Struct */
typedef struct
{
     char *name;
     void (*func)(uicb_t);
     uicb_t cmd;
     bool (*check)(uicb_t);
     char *submenu;
} MenuItem;

/* Menu Struct */
typedef struct
{
     /* Name of the menu for call
      * it in the conf (function = "menu"
      * menu = "<name>").
      */
     char *name;
     /* Placement */
     bool place_at_mouse;
     int align;
     int x, y;
     /* Color */
     struct
     {
          struct { uint bg; char *fg; } focus;
          struct { uint bg; char *fg; } normal;
     } colors;
     /* Number of item */
     int nitem, focus_item;
     /* Item */
     MenuItem *item;
} Menu;

/* Launcher struct */
typedef struct
{
     char *name;
     char *prompt;
     char *command;
     char histo[HISTOLEN][512];
     uint nhisto;
     int width;
} Launcher;

/* Button struct */
typedef struct
{
     MouseBinding *mouse;
     XSegment *linecoord;
     int nlines;
     int nmouse;
     uint flags;
} Button;

/* Alias struct */
typedef struct
{
     char *name;
     char *content;
} Alias;

/* Rule struct */
typedef struct
{
     char *class;
     char *instance;
     char *role;
     int screen;
     int tag;
     bool free;
     bool max;
     bool ignoretags;
     bool follow_client;
} Rule;

/* Configuration structure */
typedef struct
{
     /* Configuration file path */
     char confpath[512];

     /* Misc option */
     char *font;
     bool use_xft;
     uint opacity;
     bool raisefocus;
     bool focus_fmouse;
     bool focus_fmov;
     bool focus_pclick;
     bool ignore_next_client_rules;
     bool tagautohide;
     bool tagnamecount;
     Tag default_tag;
     uint pad;
     char *autostart_path;
     char *autostart_command;
     struct
     {
          /*
           * Only the colors will be use for text
           * are 'char*' (for xprint -> XftColorAllocName)
           */
          uint bar;
          char *text;
          char *tagselfg;
          char *tagurfg;
          uint tagurbg;
          uint tagselbg;
          uint tag_occupied_bg;
          char *tag_occupied_fg;
          uint tagbord;
          char *layout_fg;
          uint layout_bg;
          float client_light_shade;
          float client_dark_shade;
          float bar_light_shade;
          float bar_dark_shade;
     } colors;
     struct
     {
          int height;
          MouseBinding *mouse;
          int nmouse;
          bool selbar;
     } bars;
     struct
     {
          char *fg;
          uint bg;
          int maxlength;
          MouseBinding *mouse;
          int nmouse;
     } selbar;
     struct
     {
          char *background_command;
          MouseBinding *mouse;
          int nmouse;
     } root;
     struct
     {
          bool set_new_win_master;
          bool place_at_mouse;
          bool border_shadow;
          bool new_client_get_mouse;
          int borderheight;
          char *autofree, *automax;
          uint bordernormal;
          uint borderfocus;
          uint resizecorner_normal;
          uint resizecorner_focus;
          uint mod;
          uint padding;
          MouseBinding *mouse;
          int nmouse;
          uint default_open_tag;
          int default_open_screen;
     } client;
     struct
     {
          int height;
          char *fg_normal;
          char *fg_focus;
          struct
          {
               bool active;
               struct { uint normal, focus; } colors;
          } stipple;
          MouseBinding *mouse;
          int nmouse;
          Button *button;
          int nbutton;
     } titlebar;
     struct
     {
          bool bar;
          bool tag;
          bool layout;
     } border;
     struct
     {
          bool active;
          int screen;
          int spacing;
     } systray;
     Alias alias[256];
     uint mouse_tag_action[TagActionLast];
     int layout_button_width;
     Layout layout[NUM_OF_LAYOUT];
     Menu *menu;
     Launcher *launcher;
     Rule *rule;
     int *ntag;
     bool tag_round;
     bool tag_auto_prev;
     bool client_round;
     bool client_auto_center;
     bool client_tile_raise;
     bool layout_system; /* Switch: False, Menu: True. */
     bool layout_placement; /* Right (normal): False, Left: True. */
     bool keep_layout_geo;
     bool cfactor_enable_split;
     char *tag_expose_name;
     char *expose_layout;
     char *selected_layout_symbol;
     /* Number of... */
     int nkeybind;
     int nlayout;
     int nmenu;
     int nlauncher;
     int nrule;
} Conf;

typedef struct
{
     int as, de, width, height;
#ifdef HAVE_XFT
     XftFont *font;
#endif /* HAVE_XFT */
     XFontSet fontset;
} FontStruct;

/* status.c util struct */
typedef struct
{
     int x, y, w, h;
     uint color;
} StatusRec;

typedef struct
{
     int x, y, w, h;
     uint color;
     char data[512];
} StatusGraph;

typedef struct
{
     int x, y;
     char color[8];
     char text[512];
} StatusText;

typedef struct
{
     int x, y, w, h;
     char name[512];
} ImageAttr;

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
     char *cmd;
     char *uicb;
} vicmd_to_uicb;

typedef struct
{
     int version;
     int flags;
} xembed_info;

#endif /* STRUCTS_H */
