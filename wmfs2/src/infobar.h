/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include "wmfs.h"

enum { ElemTag = 0, ElemLayout, ElemSelbar, ElemStatus, ElemCustom };

void infobar_init(void);
Infobar *infobar_new(Scr33n *s);
void infobar_refresh(Infobar *i);
void infobar_remove(Infobar *i);
void infobar_free(Scr33n *s);

#endif /* INFOBAR_H */
