/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  File created by David Delassus.
 *  For license, see COPYING.
 */

#include "wmfs.h"
#include "util.h"

void
log_init(void)
{
     char *path = NULL;

     xasprintf(&path, "%s/wmfs-%s.log", P_tmpdir, DisplayString(W->dpy));

     if(path)
     {
          /* open log */
          if(!(W->log = fopen(path, "a")))
               warnx("Can't open log file '%s': %s\n", path, strerror(errno));

          free(path);
     }
}

/** Print a warning on standard error output and in the log file
 * \param format Format string (same syntax as printf)
 */
void
warnl(const char *format, ...)
{
     va_list args;

     if(W->log)
     {
          va_start(args, format);
          vfprintf(W->log, format, args);
          fprintf(W->log, "\n");
          va_end(args);
     }

     va_start(args, format);
     vwarn(format, args);
     va_end(args);
}

/** Print an error on standard error output and in the log file
 * \param eval Exit value
 * \param format Format string (same syntax as printf)
 */
void
errl(int eval, const char *format, ...)
{
     va_list args;

     if(W->log)
     {
          va_start(args, format);
          vfprintf(W->log, format, args);
          fprintf(W->log, "\n");
          va_end(args);
          fclose (W->log);
     }

     va_start(args, format);
     verr(eval, format, args);
}
/** Print a warning on standard error output and in the log file
 * \param format Format string (same syntax as printf)
 */
void
warnxl(const char *format, ...)
{
     va_list args;

     if(W->log)
     {
          va_start(args, format);
          vfprintf(W->log, format, args);
          fprintf(W->log, "\n");
          va_end(args);
     }

     va_start(args, format);
     vwarnx(format, args);
     va_end(args);
}

/** Print an error on standard error output and in the log file
 * \param eval Exit value
 * \param format Format string (same syntax as printf)
 */
void
errxl(int eval, const char *format, ...)
{
     va_list args;

     if(W->log)
     {
          va_start(args, format);
          vfprintf(W->log, format, args);
          fprintf(W->log, "\n");
          va_end(args);
          fclose (W->log);
     }

     va_start(args, format);
     verrx(eval, format, args);
}
