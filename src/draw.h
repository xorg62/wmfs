/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef DRAW_H
#define DRAW_H

#include <string.h>
#include <X11/Xlib.h>

#include "wmfs.h"
#include "config.h"

#define TEXTY(t, w) ((t->font.height - t->font.de) + ((w - t->font.height) >> 1))
#define PAD (8)

static inline void
draw_text(Drawable d, struct theme *t, int x, int y, Color fg, const char *str)
{
     XSetForeground(W->dpy, W->gc, fg);
     XmbDrawString(W->dpy, d, t->font.fontset, W->gc, x, y, str, strlen(str));
}

static inline void
draw_rect(Drawable d, struct geo g, Color bg)
{
     XSetForeground(W->dpy, W->gc, bg);
     XFillRectangle(W->dpy, d, W->gc, g.x, g.y, g.w, g.h);
}

/*
 * For client use
 */
static inline void
draw_reversed_rect(Drawable dr, GC gc, struct geo *g)
{
     struct geo *ug = &W->screen->ugeo;
     int i = THEME_DEFAULT->client_border_width;

     XDrawRectangle(W->dpy, dr, gc,
                    ug->x + g->x + i,
                    ug->y + g->y + i,
                    g->w - (i << 1),
                    g->h - (i << 1));
}

static inline unsigned short
draw_textw(struct theme *t, const char *str)
{
     XRectangle r;

     XmbTextExtents(t->font.fontset, str, strlen(str), NULL, &r);

     return r.width;
}

#endif /* DRAW_H */
