/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  File created by David Delassus.
 *  For license, see COPYING.
 */

#ifndef FIFO_H
#define FIFO_H

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

void fifo_init(void);
void fifo_read(void);

#endif /* FIFO_H */
