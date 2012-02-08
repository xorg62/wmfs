/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#define _GNU_SOURCE /* vasprintf() */

#include <stdint.h>
#include <string.h>

#include "util.h"

/** malloc with error support and size_t overflow protection
 * \param nmemb number of objects
 * \param size size of single object
 * \return non null void pointer
 */
void*
xmalloc(size_t nmemb, size_t size)
{
     void *ret;

     if(SIZE_MAX / nmemb < size)
          errl(EXIT_FAILURE, "xmalloc(%zu, %zu), "
                    "size_t overflow detected", nmemb, size);

     if((ret = malloc(nmemb * size)) == NULL)
          errl(EXIT_FAILURE, "malloc(%zu)", nmemb * size);

     return ret;
}

/** calloc with error support
 * \param nmemb Number of objects
 * \param size size of single object
 * \return non null void pointer
*/
void*
xcalloc(size_t nmemb, size_t size)
{
     void *ret;

     if((ret = calloc(nmemb, size)) == NULL)
          errl(EXIT_FAILURE, "calloc(%zu * %zu)", nmemb, size);

     return ret;
}

/** realloc with error support and size_t overflow check
 * \param ptr old pointer
 * \param nmemb number of objects
 * \param size size of single object
 * \return non null void pointer
 */
void *
xrealloc(void *ptr, size_t nmemb, size_t size)
{
     void *ret;

     if(SIZE_MAX / nmemb < size)
          err(EXIT_FAILURE, "xrealloc(%p, %zu, %zu), "
              "size_t overflow detected", ptr, nmemb, size);

     if((ret = realloc(ptr, nmemb * size)) == NULL)
          err(EXIT_FAILURE, "realloc(%p, %zu)", ptr, nmemb * size);

     return ret;
}


/** asprintf wrapper
 * \param strp target string
 * \param fmt format
 * \return non zero integer
 */
int
xasprintf(char **strp, const char *fmt, ...)
{
     int ret;
     va_list args;

     va_start(args, fmt);
     ret = vasprintf(strp, fmt, args);
     va_end(args);

     if (ret == -1)
          errl(EXIT_FAILURE, "asprintf(%s)", fmt);

     return ret;
}

/** strdup with error support
 * \param str char pointer
 * \retun non null void pointer
 */
char *
xstrdup(const char *str)
{
     char *ret = NULL;

     if(str == NULL || (ret = strdup(str)) == NULL)
          warnxl("strdup(%s)", str);

     return ret;
}

/** Execute a system command
 * \param cmd Command
 * \return child pid
*/
pid_t
spawn(const char *format, ...)
{
     char *sh = NULL;
     char cmd[512];
     va_list ap;
     pid_t pid;
     size_t len;

     va_start(ap, format);
     len = vsnprintf(cmd, sizeof(cmd), format, ap);
     va_end(ap);

     if (len >= sizeof(cmd))
     {
          warnxl("command too long (> 512 bytes)");
          return -1;
     }

     if(!(sh = getenv("SHELL")) || sh[0] != '/')
          sh = "/bin/sh";

     if(!(pid = fork()))
     {
          setsid();
          if (execl(sh, sh, "-c", cmd, (char*)NULL) == -1)
               warnl("execl(sh -c %s)", cmd);
          exit(EXIT_FAILURE);
     }
     else if (pid == -1)
          warnl("fork");

     return pid;
}

int
parse_args(char *str, char delim, char end, int narg, char *args[])
{
     int i = 0;

     for(args[0] = str; *str && (*str != end || *(str - 1) == '\\') && i < narg; ++str)
          if(*str == delim && i < narg - 1)
          {
               *str = '\0';
               args[++i] = ++str;
          }

     *str = '\0';

     return i;
}

void
uicb_spawn(Uicb cmd)
{
     spawn("%s", cmd);
}
