/*
*      tag.c
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
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
     Client *c;

     screen_get_sel();

     prevseltag[selscreen] = seltag[selscreen];

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
     if(tags[selscreen][prevseltag[selscreen]].barpos != tags[selscreen][seltag[selscreen]].barpos)
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

     CHECK(c);

     if(!tag)
          tag = 1;

     c->tag = tag;
     c->screen = selscreen;

     arrange(c->screen, True);

     if(c == sel && c->tag != tag)
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

/** Set the previous selected tag
  * \param cmd uicb_t type unused
  */
void
uicb_tag_prev_sel(uicb_t cmd)
{
     screen_get_sel();

     tag_set(prevseltag[selscreen]);

     return;
}

/** Transfert the selected client to the next tag
 * \param cmd uicb_t type unused
*/
void
uicb_tagtransfert_next(uicb_t cmd)
{
     CHECK(sel);
     int tag = seltag[selscreen] + 1;

     if(tag > conf.ntag[selscreen])
     {
         if(!conf.tag_round)
             return;
         tag = 1;
     }
     tag_transfert(sel, tag);

     return;
}

/** Transfert the selected client to the prev tag
 * \param cmd uicb_t type unused
*/
void
uicb_tagtransfert_prev(uicb_t cmd)
{
     CHECK(sel);
     int tag = seltag[selscreen] - 1;

     if(tag <= 0)
     {
         if(!conf.tag_round)
              return;
         tag = conf.ntag[selscreen];
     }
     tag_transfert(sel, tag);

     return;
}

/** Go to the current urgent tag
  *\param cmd uicb_t type unused
  */
void
uicb_tag_urgent(uicb_t cmd)
{
     Client *c;
     Bool b = False;

     /* Check if there is a urgent client */
     for(c = clients; c; c = c->next)
          if(c->flags & UrgentFlag)
          {
               b = True;
               break;
          }

     if(!b)
          return;

    screen_set_sel(c->screen);
    tag_set(c->tag);
    client_focus(c);

    return;
}

/** Add a additional tag to the current tag
  *\param sc Screen
  *\param tag Tag where apply additional tag
  *\param adtag Additional tag to apply in tag
 */
void
tag_additional(int sc, int tag, int adtag)
{
     if(tag < 0 || tag > conf.ntag[sc]
       || adtag < 1 || adtag > conf.ntag[sc] || adtag == seltag[sc])
          return;

     tags[sc][tag].tagad ^= TagFlag(adtag);
     tags[sc][adtag].request_update = True;
     arrange(sc, True);

     return;
}

/** Add a additional tag to the current tag
  *\param cmd uicb_t
 */
void
uicb_tag_toggle_additional(uicb_t cmd)
{
     screen_get_sel();

     tag_additional(selscreen, seltag[selscreen], atoi(cmd));

     return;
}



