/*
*  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
*  File created by David Delassus.
*  For license, see COPYING.
*/

#ifndef __LOG_H
#define __LOG_H

#include <errno.h>

void log_init(void);
void warnl(const char *format, ...);
void warnxl(const char *format, ...);
void errl(int eval, const char *format, ...);
void errxl(int eval, const char *format, ...);

#endif /* __LOG_H */
