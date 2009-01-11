#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#define SIZE 512

#define PROMPT "wmfs-shell >> "
#define CLEAR  "\033[H\033[2J"

#define HELPSTR                                         \
"\n      help, list                       Print this message.\n\
      clear                            Clear the terminal.\n\
      exit, quit                       Quit wmfs-shell.\n\
      uicb_list                        Print all uicb wmfs function.\n\
      exec(<uicb func name>, <arg>)    Execute a Wmfs uicb function.\n\
      spawn(<command>)                 Execute a system command.\n\n"
#define UICBLIST \
     "Here is a list of all wmfs's uicb functions that are usable with the exec function:   \n\
     spawn                         Exec a system command.                                   \n\
     client_kill                   Kill the selected client.                                \n\
     client_prev                   Select the previous client.                              \n\
     client_next                   Select the next client.                                  \n\
     toggle_max                    Toggle the selected client max/<current layout>.         \n\
     layout_next                   Set the next layout.                                     \n\
     layout_prev                   Set the previous layout.                                 \n\
     tag                           Manipul tag (1, 3, +2 ..).                               \n\
     tag_next                      Select the next tag.                                     \n\
     tag_prev                      Select the previous tag.                                 \n\
     tag_transfert                 Transfert the selected client to the tag.                \n\
     set_mwfact                    Set the mwfact (+0.10, -0.43, ...).                      \n\
     set_nmaster                   Set the nmaster (+1 ..).                                 \n\
     quit                          Quit WMFS.                                               \n\
     toggle_infobar_position       Toggle the infobar position (top/bottom).                \n\
     mouse_move                    Select the selected client with the mouse and move it.   \n\
     mouse_resize                  Select the selected client with the mouse and resize it. \n\
     client_raise                  Raise the selected client.                               \n\
     tile_switch                   Switch the master client in the tile grid.               \n\
     toggle_free                   Toggle the selected client to free.                      \n\
     screen_select                 Select the screen.                                       \n\
     screen_next                   Select the next screen.                                  \n\
     screen_prev                   Select the previous screen.                              \n\
     reload                        Reload the WMFS configuration.                           \n\n"


/* Xlib util macro */
#define ROOT    RootWindow(dpy, SCREEN)
#define ATOM(a) XInternAtom(dpy, a, False)
#define SCREEN  DefaultScreen(dpy)

/* Protos */
void send_client_message(char* atom_name, long data_l[5]);
void exec_uicb_function(char *func, char *cmd);
void manage_input(char *input);

/* Variables */
Display *dpy;
