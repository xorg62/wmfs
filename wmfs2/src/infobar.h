/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef INFOBAR_H
#define INFOBAR_H

#include "wmfs.h"

enum { ElemTag = 0, ElemLayout, ElemSelbar, ElemStatus, ElemCustom };
/*
const struct elem_funcs { char c; void (*func_init)(Infobar *i); void (*func_update)(Element *e); } elem_funcs[] =
{
     { 't',  infobar_elem_tag_init,    infobar_elem_tag_update },
     { 'l',  infobar_elem_layout_init, infobar_elem_layout_update },
     { 's',  infobar_elem_selbar_init, infobar_elem_selbar_update },
     { 'S',  infobar_elem_status_init, infobar_elem_status_update },
     { '\0', NULL, NULL }
};*/

void infobar_init(void);
Infobar *infobar_new(Scr33n *s);
void infobar_refresh(Infobar *i);
void infobar_remove(Infobar *i);
void infobar_free(Scr33n *s);

#endif /* INFOBAR_H */
