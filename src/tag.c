/*
*      tag.c
*      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "wmfs.h"

/* Set the tag
 * \param tag The tag number
*/
void
tag_set(int tag)
{
     int otag;
     Client *c;

     screen_get_sel();

     otag = seltag[selscreen];

     if(conf.tag_round)
     {
          if(tag <= 0)
               seltag[selscreen] = conf.ntag[selscreen];
          else if(tag  > conf.ntag[selscreen])
               seltag[selscreen] = 1;
          else
               seltag[selscreen] = tag;
     }
     else
     {
          if(!tag || tag == seltag[selscreen]
             || tag > conf.ntag[selscreen])
               return;
          seltag[selscreen] = tag;
     }

     ewmh_update_current_tag_prop();

     /* Arrange infobar position */
     if(tags[selscreen][otag].barpos != tags[selscreen][seltag[selscreen]].barpos)
          infobar_set_position(tags[selscreen][seltag[selscreen]].barpos);

     arrange(selscreen, False);

     if(tags[selscreen][tag].request_update)
     {
          tags[selscreen][seltag[selscreen]].layout.func(selscreen);
          tags[selscreen][tag].request_update = False;
     }

     /* To focus the first client in the new tag */
     for(c = clients; c; c = c->next)
          if(c->tag == seltag[selscreen] && c->screen == selscreen)
               break;

     client_focus((c) ? c : NULL);

     return;
}

/* Transfert a client to a tag
 * \param c Client pointer
 * \param tag Tag
*/
void
tag_transfert(Client *c, int tag)
{
     screen_get_sel();

     if(!c || c->tag == tag)
          return;

     if(!tag)
          tag = 1;

     c->tag = tag;

     arrange(c->screen, True);

     if(c == sel)
          client_focus(NULL);

     client_update_attributes(c);

     tags[c->screen][tag].request_update = True;

     return;
}

/** Uicb Set a tag
 * \param cmd Tag number or '+' / '-', uicb_t type
*/
void
uicb_tag(uicb_t cmd)
{
     int tmp = atoi(cmd);

     if(cmd[0] == '+' || cmd[0] == '-')
          tag_set(seltag[selscreen] + tmp);
     else
          tag_set(tmp);

     return;
}

/** Set the next tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_next(uicb_t cmd)
{
     screen_get_sel();

     tag_set(seltag[selscreen] + 1);

     return;
}

/** Set the previous tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_prev(uicb_t cmd)
{
     screen_get_sel();

     tag_set(seltag[selscreen] - 1);

     return;
}

/** Transfert the selected client to
 *  the wanted tag
 * \param cmd Wanted tag, uicb_t type
*/
void
uicb_tagtransfert(uicb_t cmd)
{
     CHECK(sel);

     tag_transfert(sel, atoi(cmd));

     return;
}
