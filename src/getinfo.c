/*
*      getinfo.c
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

/* Global variables for each XGetWindowProperty
 * of each getinfo functions.
 */
Atom rt;
int rf;
ulong ir, il;
uchar *ret;

/** Get information about tag (current, list, names)
 */
void
getinfo_tag(void)
{
     int tag = 0;
     char *tag_name = NULL;
     char *tag_list = NULL;


     if(XGetWindowProperty(dpy, ROOT, ATOM("_NET_CURRENT_DESKTOP"), 0L, 4096,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          tag = (int)*ret + 1;
          XFree(ret);
     }

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_CURRENT_TAG"), 0L, 4096,
                           False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          tag_name = _strdup((char*)ret);
          XFree(ret);
     }

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_TAG_LIST"), 0L, 4096,
                           False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          tag_list = _strdup((char*)ret);
          XFree(ret);
     }

     printf("Current tag:  %d - %s\n", tag, tag_name);
     printf("Tag list:  %s\n", tag_list);

     IFREE(tag_name);
     IFREE(tag_list);

     return;
}

/** Get information about screens
 */
void
getinfo_screen(void)
{
     int screen = 1;
     int screen_num = 1;

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_CURRENT_SCREEN"), 0L, 4096,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          screen = (int)*ret + 1;
          XFree(ret);
     }
     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_SCREEN_COUNT"), 0L, 4096,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          screen_num = (int)*ret;
          XFree(ret);
     }

     printf("Current screen:  %d\nScreen number:  %d\n", screen, screen_num);

     return;
}

/** Get current layout name
 */
void
getinfo_layout(void)
{
     char *layout = NULL;

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_CURRENT_LAYOUT"), 0L, 4096,
                           False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          layout = _strdup((char*)ret);
          XFree(ret);
     }

     printf("Current layout:  %s\n", layout);

     IFREE(layout);

     return;
}

/** Get information about current mwfact
 */
void
getinfo_mwfact(void)
{
     char *mwfact = NULL;

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_MWFACT"), 0L, 4096,
                           False, XA_STRING, &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          mwfact = _strdup((char*)ret);
          XFree(ret);
     }

     printf("Current mwfact:  %s\n", mwfact);

     IFREE(mwfact);

     return;
}

/** Get information about current nmaster
 */
void
getinfo_nmaster(void)
{
     int nmaster = 1;

     if(XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_NMASTER"), 0L, 4096,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success && ret)
     {
          nmaster = (int)*ret;
          XFree(ret);
     }

     printf("Current nmaster:  %d\n", nmaster);

     return;
}

/** Get information about wmfs
 *\param info Type of information in a string
 */
void
getinfo(char *info)
{
     long data[5];

     if(!check_wmfs_running())
          return;

     data[4] = True;

     send_client_event(data, "_WMFS_UPDATE_HINTS");

     if(!strcmp(info, "tag"))
          getinfo_tag();
     else if(!strcmp(info, "screen"))
          getinfo_screen();
     else if(!strcmp(info, "layout"))
          getinfo_layout();
     else if(!strcmp(info, "mwfact"))
          getinfo_mwfact();
     else if(!strcmp(info, "nmaster"))
          getinfo_nmaster();
     else if(!strcmp(info, "help"))
          printf("Argument list for wmfs -g options:\n"
                 "    tag            Show current tag number and name, and tag list.\n"
                 "    screen         Show current screen and screens number.\n"
                 "    layout         Show current layout name.\n"
                 "    mwfact         Show mwfact of current tag.\n"
                 "    nmaster        Show nmaster of current tag.\n");
     else
          warnx("Unknow info argument '%s'\nTry 'wmfs -g help'", info);

     return;
}

