/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef UTIL_H
#define UTIL_H

#include "wmfs.h"

#include <string.h>

/* Todo FREE_LIST(type, head, function_remove) */
#define FREE_LIST(type, head)                   \
     do {                                       \
          struct type *Z;                       \
          while(!SLIST_EMPTY(&head)) {          \
               Z = SLIST_FIRST(&head);          \
               SLIST_REMOVE_HEAD(&head, next);  \
               free(Z); /* function_remove(t)*/ \
          }                                     \
     } while(/* CONSTCOND */ 0);

/* Insert at the end with SLIST */
#define SLIST_INSERT_TAIL(head, elem, field, prev) \
     if(!prev)                                     \
          SLIST_INSERT_HEAD(head, elem, field);    \
     else                                          \
          SLIST_INSERT_AFTER(prev, elem, field);

/* t is Map or Unmap */
#define WIN_STATE(w, t) do {      \
     X##t##Subwindows(W->dpy, w); \
     X##t##Window(W->dpy, w);     \
} while( /* CONSTCOND */ 0);

#define ATOM(a)             XInternAtom(W->dpy, (a), False)
#define LEN(x)              (sizeof(x) / sizeof(*x))
#define FLAGINT(i)          (1 << i)
#define FLAGAPPLY(f, b, m)  (f |= (b ? m : 0))
#define ATOI(s)             strtol(s, NULL, 10)
#define ABS(j)              (j < 0 ? -j : j)
#define INAREA(i, j, a)     ((i) >= (a).x && (i) <= (a).x + (a).w && (j) >= (a).y && (j) <= (a).y + (a).h)
#define GEOCMP(g1, g2)      ((g1).x == (g2).x && (g1).y == (g2).y && (g1).w == (g2).w && (g1).h == (g2).h)

/*
 * "#RRGGBB" -> 0xRRGGBB
 */
static inline Color
color_atoh(const char *col)
{
     XColor xcolor;

     if(!XAllocNamedColor(W->dpy, DefaultColormap(W->dpy, W->xscreen), col, &xcolor, &xcolor))
          warnl("Error: cannot allocate color \"%s\".", col);

     return xcolor.pixel;
}

#ifdef HAVE_XFT
static inline XftColor
xftcolor_atoh(const char *col)
{
     XftColor xcolor;

     if (!XftColorAllocName(W->dpy, DefaultVisual(W->dpy, W->xscreen), DefaultColormap(W->dpy, W->xscreen), col, &xcolor))
          warnl("Error: cannot allocate color \"%s\".", col);

     return xcolor;
}
#endif /* HAVE_XFT */

#ifdef HAVE_XFT
#define fgcolor_atoh xftcolor_atoh
#define bgcolor_atoh color_atoh
#else
#define fgcolor_atoh color_atoh
#define bgcolor_atoh color_atoh
#endif /* HAVE_XFT */

static inline void
swap_ptr(void **x, void **y)
{
     void *t = *x;

     *x = *y;
     *y = t;
}

static inline void
swap_int(int *x, int *y)
{
     *y = *x ^ *y;
     *x = *y ^ *x;
     *y = *x ^ *y;
}

static inline enum position
str_to_position(char *str)
{
     enum position i;
     static const char index[PositionLast][8] = { "right", "left", "top", "bottom", "center" };

     for(i = 0; i < PositionLast; ++i)
          if(!strcmp(index[i], str))
               return i;

     return Right;
}

void *xmalloc(size_t nmemb, size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t nmemb, size_t size);
int xasprintf(char **strp, const char *fmt, ...);
char *xstrdup(const char *str);
pid_t spawn(const char *format, ...);
int parse_args(char *str, char delim, char end, int narg, char *args[]);
void uicb_spawn(Uicb cmd);

#endif /* UTIL_H */
