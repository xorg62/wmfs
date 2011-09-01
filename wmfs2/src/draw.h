/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef DRAW_H
#define DRAW_H

#include <string.h>
#include <X11/Xlib.h>

#include "wmfs.h"

#define TEXTY(t, w) ((t->font.height - t->font.de) + ((w - t->font.height) >> 1))
#define PAD (8)

static inline void
draw_text(Drawable d, Theme *t, int x, int y, Color fg, const char *str)
{
     XSetForeground(W->dpy, W->gc, fg);
     XmbDrawString(W->dpy, d, t->font.fontset, W->gc, x, y, str, strlen(str));
}

static inline unsigned short
draw_textw(Theme *t, const char *str)
{
     XRectangle r;

     XmbTextExtents(t->font.fontset, str, strlen(str), NULL, &r);

     return r.width;
}

#endif /* DRAW_H */
