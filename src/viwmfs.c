/*
*      viwmfs.c
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

vicmd_to_uicb vicmd[] =
{
     {"r",       "reload"},
     {"q",       "quit"},
     {"t",       "tag"},
     {"tn",      "tag_next"},
     {"tp",      "tag_prev"},
     {"l",       "set_layout"},
     {"layout",  "set_layout"},
     {"ln",      "layout_next"},
     {"lp",      "layout_prev"},
     {"s",       "screen_select"},
     {"screen",  "screen_select"},
     {"sn",      "screen_next"},
     {"sp",      "screen_prev"},
     {"cc",      "client_kill"},
     {"ct",      "tag_transfert"},
     {"cn",      "client_next"},
     {"cp",      "client_prev"},
     {"csn",     "client_swap_next"},
     {"csp",     "client_swap_prev"},
     {"mwf",     "set_mwfact"},
     {"mwfact",  "set_mwfact"},
     {"nmaster", "set_nmaster"},
     {"tm",      "toggle_max"},
     {"tf",      "toggle_free"},
};

void
viwmfs_help(void)
{
     int i;
     char s[20];

     printf("ViWMFS commands list:\n");

     for(i = 0; i < LEN(vicmd); ++i)
     {
          memset(s, ' ', sizeof(s));

          s[15 - strlen(vicmd[i].cmd)] = '\0';

          printf("   :%s%s %s\n", vicmd[i].cmd, s, vicmd[i].uicb);
     }

     return;
}

void
viwmfs(int argc, char **argv)
{
     int i;
     char *cmd, str[256] = { 0 };
     Bool e;

     if(!argc)
          return;

     if(argc > 3)
     {
          for(i = 2; i < argc; ++i)
          {
               strcat(str, argv[i]);

               if(*(str + strlen(str) - 1) != ':')
                    strcat(str, " ");
          }
     }
     else
          strcpy(str, argv[2]);

     if(!strcmp(str, "help"))
     {
          viwmfs_help();

          return;
     }

     if(*str == ':')
     {
          for(i = 0; i < strlen(str); str[i] = str[i + 1], ++i);

          cmd = _strdup(str);

          for(i = 0; cmd[i] && cmd[i] != ' '; ++i);
          cmd[i] = '\0';

          for(i = 0; i < LEN(vicmd); ++i)
               if(!strncmp(cmd, vicmd[i].cmd, strlen(cmd)))
               {
                    exec_uicb_function(vicmd[i].uicb, clean_value(str + strlen(cmd)));
                    e = True;
                    break;
               }

          /* For uicb function with normal name specified */
          if(!e)
          {
               if(!strcmp(str, "h") || !strcmp(str, "help"))
                    viwmfs_help();
               else
                    exec_uicb_function(cmd, str + strlen(cmd));
          }
     }

     return;
}
