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

/** Set a tag
 * \param cmd Tag number or '+' / '-', uicb_t type
*/
void
uicb_tag(uicb_t cmd)
{
     int tmp = atoi(cmd);

     if(!tmp)
          tmp = 1;

     if(cmd[0] == '+' || cmd[0] == '-')
     {
          if(tmp + seltag < 1
             || tmp + seltag > conf.ntag)
               return;
          prevtag = seltag;
          seltag += tmp;
     }
     else
     {
          if(tmp == seltag
             || tmp > conf.ntag)
               return;
          prevtag = seltag;
          seltag = tmp;
     }
     arrange();

     return;
}

/** Set the next tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_next(uicb_t cmd)
{
     uicb_tag("+1");

     return;
}

/** Set the previous tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_prev(uicb_t cmd)
{
     uicb_tag("-1");

     return;
}

/** Transfert the selected client to
 *  the wanted tag
 * \param cmd Wanted tag, uicb_t type
*/
void
uicb_tagtransfert(uicb_t cmd)
{
     int n = atoi(cmd);

     if(!sel || n == seltag)
          return;

     if(!n)
          n = 1;

     sel->tag = n;
     selbytag[n] = sel;
     selbytag[seltag] = NULL;

     arrange();

     return;
}
