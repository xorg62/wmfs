#include "wmfs-shell.h"

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
     char  *sh;

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
manage_input(char *input)
{
     char *p = input;
     char *q1, *q2, *tmp, *aargs = NULL, *func = input;
     char **args;
     int i, v = 0;


     if(!strcmp(input, "clear"))
          printf(CLEAR);
     else if(!strcmp(input, "exit")
        || !strcmp(input, "quit"))
          exit(EXIT_FAILURE);
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
          else if(!strcmp(func, "spawn"))
          {
               if(v > 0 || !args[0])
                    printf("Spawn: spawn(<command>), Execute a system command.\n");
               else
                    spawn(args[0]);
          }

          free(args);
     }


     return;
}

int
main(void)
{
     char *input, *p, c;

     if(!(dpy = XOpenDisplay(NULL)))
          fprintf(stderr, "wmfs-shell: Wmfs is probably not running: cannot open X server. \n");

     for(;;)
     {
          printf(PROMPT);

          input = malloc(sizeof(char) * SIZE);

          fgets(input, SIZE, stdin);

          if((p = strrchr(input, '\n')) != NULL)
               *p = '\0';
          else
               while((c = fgetc(stdin)) != '\n' && c != EOF);

          manage_input(input);

          free(input);
     }



     return 0;
}
