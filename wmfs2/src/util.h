/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef UTIL_H
#define UTIL_H

#include "wmfs.h"

/* Todo FREE_LIST(type, head, function_remove) */
#define FREE_LIST(type, head)                   \
     do {                                       \
          struct type *t;                       \
          while(!SLIST_EMPTY(&head)) {          \
               t = SLIST_FIRST(&head);          \
               SLIST_REMOVE_HEAD(&head, next);  \
               free(t); /* function_remove(t)*/ \
          }                                     \
     } while(/* CONSTCOND */ 0);

/* t is Map or Unmap */
#define WIN_STATE(w, t) do {      \
     X##t##Subwindows(W->dpy, w); \
     X##t##Window(W->dpy, w);     \
} while( /* CONSTCOND */ 0);


#define ATOM(a)          XInternAtom(W->dpy, (a), False)
#define LEN(x)           (sizeof(x) / sizeof(*x))
#define FLAGINT(i)       (1 << i)
#define ATOI(s)          strtol(s, NULL, 10)
#define ABS(j)           (j < 0 ? -j : j)
#define INAREA(i, j, a)  ((i) >= (a).x && (i) <= (a).x + (a).w && (j) >= (a).y && (j) <= (a).y + (a).h)


/*
 * "#RRGGBB" -> 0xRRGGBB
 */
static inline Color
color_atoh(const char *col)
{
     int shift = (col[0] == '#');

     return (Color)strtol(col + shift, NULL, 16);
}

void *xmalloc(size_t nmemb, size_t size);
void *xcalloc(size_t nmemb, size_t size);
int xasprintf(char **strp, const char *fmt, ...);
char *xstrdup(const char *str);
pid_t spawn(const char *format, ...);
void uicb_spawn(Uicb cmd);

#endif /* UTIL_H */
