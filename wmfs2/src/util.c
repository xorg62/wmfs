/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "wmfs.h"
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
          err(EXIT_FAILURE, "xmalloc(%zu, %zu), "
                    "size_t overflow detected", nmemb, size);

     if((ret = malloc(nmemb * size)) == NULL)
          err(EXIT_FAILURE, "malloc(%zu)", nmemb * size);

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
          err(EXIT_FAILURE, "calloc(%zu * %zu)", nmemb, size);

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
          warnx("command too long (> 512 bytes)");
          return -1;
     }

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";

     if((pid = fork()) == 0)
     {
          setsid();
          if (execl(sh, sh, "-c", cmd, (char*)NULL) == -1)
               warn("execl(sh -c %s)", cmd);
          exit(EXIT_FAILURE);
     }
     else if (pid == -1)
          warn("fork");

     return pid;
}
