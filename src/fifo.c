/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  File created by David Delassus.
 *  For license, see COPYING.
 */

#include <sys/stat.h> /* access */

#include "wmfs.h"
#include "util.h"
#include "config.h"
#include "fifo.h"

#define READ_SIZE (32768)

static void
fifo_open(void)
{
     if(W->fifo.fd)
          close(W->fifo.fd);

     if(!(W->fifo.fd = open(W->fifo.path, O_RDONLY | O_NDELAY, 0)))
          warnxl("Can't open FIFO: %s\n", strerror(errno));
}

void
fifo_init(void)
{
     xasprintf(&(W->fifo.path), "%s/wmfs-%s.fifo", P_tmpdir, DisplayString(W->dpy));

     /* Check if fifo already exists */
     if(access(W->fifo.path, F_OK) != -1)
          unlink(W->fifo.path);

     if(mkfifo(W->fifo.path, 0644) < 0)
          warnxl("Can't create FIFO: %s\n", strerror(errno));

     fifo_open();
}

static void
fifo_parse(char *cmd)
{
     void (*func)(Uicb);
     char *p, *arg = NULL;

     /* remove trailing newline */
     if((p = strchr(cmd, '\n')))
          *p = '\0';

     /* If an argument is present, delimit function string */
     if((p = strchr(cmd, ' ')))
     {
          *p = '\0';
          arg = p + 1;
     }

     /* call the UICB function, p + 1 is command or NULL */
     if((func = uicb_name_func(cmd)))
          func(arg);

     XSync(W->dpy, false);
}

void
fifo_read(void)
{
     char buf[READ_SIZE] = { 0 };
     int ret;

     if((ret = read(W->fifo.fd, buf, sizeof(buf) - 1)) > 0)
          fifo_parse(buf);
     else if(!ret)
          fifo_open();
}
