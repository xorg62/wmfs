/*
*      wmfs-shell.c
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

#include "wmfs-shell.h"

void
init(void)
{
     Atom rt;
     int rf;
     unsigned long ir, il;
     unsigned char *ret;

     /* Init display */
     if(!(dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "wmfs-shell: Wmfs is probably not running: cannot open X server. \n");
          exit(EXIT_SUCCESS);
     }

     /* Check if wmfs is running */
     XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_RUNNING"), 0L, 4096,
                        False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret);

     if(!ret)
     {
          XFree(ret);
          fprintf(stderr, "Wmfs is not running. ( _WMFS_RUNNING not present)\n");
          exit(EXIT_FAILURE);
     }

     return;
}

void
send_client_message(char* atom_name, long data_l[5])
{
     XEvent ev;
     int i;

     ev.xclient.type = ClientMessage;
     ev.xclient.serial = 0;
     ev.xclient.send_event = True;
     ev.xclient.message_type = ATOM(atom_name);
     ev.xclient.window = ROOT;
     ev.xclient.format = 32;

     for(i = 0; i < 6; ++i)
          ev.xclient.data.l[i] = data_l[i];

     XSendEvent(dpy, ROOT, False, SubstructureRedirectMask|SubstructureNotifyMask, &ev);
     XSync(dpy, False);

     return;
}

void
spawn(char* arg)
{
     char *sh;

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";
     if(!strlen(arg))
          return;

     if(fork() == 0)
     {
          if(fork() == 0)
          {
               setsid();
               execl(sh, sh, "-c", arg, (char*)NULL);
          }
          exit(EXIT_SUCCESS);
     }

     return;
}

void
statustext(char *text)
{
     long data[5];

     data[4] = True;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_STATUSTEXT"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (unsigned char*)text, strlen(text));

     send_client_message("_WMFS_STATUSTEXT", data);

     return;
}


void
exec_uicb_function(char *func, char *cmd)
{
     long data[5];

     data[4] = True;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_FUNCTION"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (unsigned char*)func, strlen(func));

     if(cmd == NULL)
          cmd = "";
     XChangeProperty(dpy, ROOT, ATOM("_WMFS_CMD"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (unsigned char*)cmd, strlen(cmd));

     send_client_message("_WMFS_FUNCTION", data);

     return;
}

void
tag_get_current(char **name, int *num)
{
     Atom rt;
     int rf;
     unsigned long ir, il;
     unsigned char *ret;

     if(name && XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_CURRENT_TAG"), 0L, 4096,
                                   False, ATOM("UTF8_STRING"), &rt, &rf, &ir, &il, &ret) == Success)
          *name = (char*)ret;

     if(XGetWindowProperty(dpy, ROOT, ATOM("_NET_CURRENT_DESKTOP"), 0L, 4096,
                           False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret) == Success)
          *num = *(int*)ret;

     if(ret)
          XFree(ret);

     return;
}

void
tag_set(int tag)
{
     long data[5];
     int t;
     char *n;

     if(tag >= 0)
          data[0] = tag;

     send_client_message("_NET_CURRENT_DESKTOP", data);

     usleep(100000); /* For tag_get_current */

     tag_get_current(&n, &t);
     printf("Your now on '%s' (tag %d).\n", n, t);

     return;
}

void
manage_input(char *input)
{
     char *p = input, *func = input;
     char *q1, *q2, *tmp, *aargs = NULL;
     char **args;
     int i, v = 0;

     if(!strcmp(input, "clear"))
          printf(CLEAR);
     else if(!strcmp(input, "exit")
        || !strcmp(input, "quit"))
          exit(EXIT_SUCCESS);
     else if(!strcmp(input, "help")
             || !strcmp(input, "list"))
          printf(HELPSTR);
     else if(!strcmp(input, "uicb_list")
             || !strcmp(input, "uicb_function_list"))
          printf(UICBLIST);

     /* If there is a function ( func() ) */
     else if((q1 = strchr(p, '('))
             && (q2 = strchr(p, ')'))
             && p[0] != '(')
     {
          /* Set FUNCTION Name AND ARGS in **args
           * {{{ */

          /* Get function name */
          for(i = 0; i < strlen(p) - ((strlen(p) - strlen(q1))); ++i)
               func[ (strlen(p) - strlen(q1)) + i ] = 0;

          *q1 = *q2 = '\0';
          aargs = q1 + 1;

          /* Count how many ',' there is in aargs */
          for(i = 0; i < strlen(aargs) + 1; ++i)
               if(aargs[i] == ',')
                    ++v;

          args = malloc(sizeof(char*) * v + 1);
          tmp = strtok(aargs, ",");

          for(i = 0; tmp != NULL; ++i)
          {
               args[i] = tmp;
               tmp = strtok(NULL, ",");
          }
          /*
           * }}} */

          /* Manage function */

          if(!strcmp(func, "exec"))
          {
               if(v > 1 || !args[0])
                    printf("Exec: exec(<uicb function name>, <cmd or nothing>).\n");
               else
               {
                    if(!args[1])
                         args[1] = NULL;
                    exec_uicb_function(args[0], args[1]);
               }
          }
          else if(!strcmp(func, "tag_set"))
          {
               if(v > 0 || !args[0])
                    printf("Tag_set: tag_set(<tag_number>), Set the current tag.\n");
               else
                    tag_set(atoi(args[0]));
          }
          else if(!strcmp(func, "statustext"))
          {
               if(v > 0 || !args[0])
                    printf("Statustext: statustext(<text>), Print text in the wmfs bar.\n");
               else
                    statustext(args[0]);
          }
          else if(!strcmp(func, "spawn"))
          {
               if(v > 0 || !args[0])
                    printf("Spawn: spawn(<command>), Execute a system command.\n");
               else
                    spawn(args[0]);
          }

          free(args);
     }
     else
          printf("Unknow command '%s', try 'help'.\n", input);

     return;
}

int
main(int argc, char *argv[])
{
     char *input, *p;
     int c;
     char opt;

     static struct option shell_opts[] =
     {
        {"help",  0, NULL, 'h'},
        {"cmd",   1, NULL, 'c'},
        {0}
     };

     /* Initialisation */
     init();

     /* get args from argv */
     while(EOF != (opt = (char)getopt_long(argc, argv, "hc", shell_opts, NULL)))
     {

        /* Just print help */
        if( (opt == 'h') || (opt == '?'))
        {
           printf("Usage : wmfs-shell [-c|--cmd cmd]\n");
           exit(0);
        }

        else if(opt == 'c')
        {
           /* Run a single command */
           if((argc >= 2)&&(argv[2] != '\0'))
              manage_input(argv[2]);
           else
              printf("Run 'wmfs-shell --help' for help\n");
           exit(0);
        }

     }

     for(;;)
     {
          printf(PROMPT);

          input = malloc(sizeof(char) * SIZE);

          fgets(input, SIZE, stdin);

          if((p = strrchr(input, '\n')) != NULL)
               *p = '\0';
          else
               while((c = fgetc(stdin)) != '\n' && c != EOF);

          if(*input != '\0')
               manage_input(input);

          free(input);
     }

     return 0;
}
