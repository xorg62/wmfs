/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  File created by David Delassus.
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "fifo.h"
#include "config.h"

#define FIFO_DEFAULT_PATH     "/tmp/wmfs-fifo"

void
fifo_init(void)
{
     W->fifo.path = FIFO_DEFAULT_PATH;

     if(mkfifo(W->fifo.path, 0644) < 0
               || !(W->fifo.fd = open(W->fifo.path, O_NONBLOCK, 0)))
     {
          warnx("Can't create FIFO: %s\n", strerror(errno));
          return;
     }
}

static void
fifo_parse(char *cmd)
{
     void (*func)(Uicb cmd);
     char *uicb = strtok(cmd, " ");
     char *arg  = strtok(NULL, "\n");

     printf("%s %s\n", uicb, arg);

     func = uicb_name_func(uicb);
     if(func)
          func(arg);

     XSync(W->dpy, false);
}

void
fifo_read(void)
{
     char buf[256] = {0};

     /* Don't read it if not open */
     if(!(W->fifo.fd))
          return;

     read(W->fifo.fd, buf, sizeof(buf) - 1);

     if(buf[0])
          fifo_parse(buf);
}

