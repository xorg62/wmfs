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
     Bool al = False;
     int i;

     if(tag < 0 || tag > MAXTAG)
          return;

     screen_get_sel();

     if(seltag[selscreen] != tag)
          prevseltag[selscreen] = seltag[selscreen];
     else if(tag == seltag[selscreen] && tag != prevseltag[selscreen] && conf.tag_auto_prev)
          tag = seltag[selscreen] = prevseltag[selscreen];
     else
          seltag[selscreen] = tag;

     if(conf.tag_round)
     {
          if(tag <= 0)
               seltag[selscreen] = conf.ntag[selscreen];
          else if(tag > conf.ntag[selscreen])
               seltag[selscreen] = 1;
          else
               seltag[selscreen] = tag;
     }
     else
     {
          if(!tag || tag > conf.ntag[selscreen])
               return;

          seltag[selscreen] = tag;
     }

     ewmh_update_current_tag_prop();

     /* Arrange infobar position */
     if(tags[selscreen][prevseltag[selscreen]].barpos != tags[selscreen][seltag[selscreen]].barpos
               || prevseltag[selscreen] == seltag[selscreen])
          infobar_set_position(tags[selscreen][seltag[selscreen]].barpos);

     /* Check if a layout update is needed with additional tags */
     if(tags[selscreen][seltag[selscreen]].tagad)
          al = True;
     else if(tags[selscreen][seltag[selscreen]].request_update)
     {
          al = True;
          tags[selscreen][seltag[selscreen]].request_update = False;
     }

     for(i = 1; i < conf.ntag[selscreen] + 1; ++i)
          if(tags[selscreen][i].tagad & TagFlag(seltag[selscreen]))
          {
               al = True;
               break;
          }

     /* Check for ignore_tag clients */
     for(c = clients; c; c = c->next)
          if(c->tag == MAXTAG + 1 && c->screen == selscreen)
          {
               al = True;
               break;
          }

     arrange(selscreen, al);

     if(tags[selscreen][tag].request_update)
     {
          tags[selscreen][seltag[selscreen]].layout.func(selscreen);
          tags[selscreen][tag].request_update = False;
     }

     /* To focus selected client of the via focusontag option */
     for(c = clients; c; c = c->next)
          if(c->focusontag == tag && c->screen == selscreen)
               break;

     /* No focusontag option found on any client, try to find the first of the tag */
     if(!c)
          for(c = clients; c; c = c->next)
               if(c->tag == (uint)seltag[selscreen] && c->screen == selscreen)
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

     if(tag <= 0)
          tag = 1;

     if(tag > conf.ntag[selscreen])
          return;

     c->tag = tag;
     c->screen = selscreen;

     arrange(c->screen, True);

     client_focus_next(c);

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
     (void)cmd;
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
     (void)cmd;
     screen_get_sel();

     tag_set(seltag[selscreen] - 1);

     return;
}

/** Set the next visible tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_next_visible(uicb_t cmd)
{
     int i, tag;
     Client *c;
     Bool is_occupied[MAXTAG];
     (void)cmd;

     screen_get_sel();

     if(!conf.tagautohide)
     {
          tag_set(seltag[selscreen] + 1);
          return;
     }

     for(i = 0; i < MAXTAG; i++)
          is_occupied[i] = False;

     for(c = clients; c; c = c->next)
          if(c->screen == selscreen)
               is_occupied[c->tag] = True;

     for(tag = seltag[selscreen] + 1; tag < conf.ntag[selscreen] + 1; ++tag)
          if(is_occupied[tag])
          {
               tag_set(tag);
               return;
          }

     if(conf.tag_round)
          for(tag = 0; tag < seltag[selscreen]; ++tag)
               if(is_occupied[tag])
               {
                    tag_set(tag);
                    return;
               }

     return;
}

/** Set the prev visible tag
 * \param cmd uicb_t type unused
*/
void
uicb_tag_prev_visible(uicb_t cmd)
{
     int i, tag;
     Client *c;
     Bool is_occupied[MAXTAG];
     (void)cmd;

     screen_get_sel();

     if(!conf.tagautohide)
     {
          tag_set(seltag[selscreen] - 1);
          return;
     }

     for(i = 0; i < MAXTAG; i++)
          is_occupied[i] = False;

     for(c = clients; c; c = c->next)
          if(c->screen == selscreen)
               is_occupied[c->tag] = True;

     for(tag = seltag[selscreen] - 1; tag >= 0; --tag)
          if(is_occupied[tag])
          {
               tag_set(tag);
               return;
          }

     if(conf.tag_round)
          for(tag = conf.ntag[selscreen]; tag > seltag[selscreen]; --tag)
               if(is_occupied[tag])
               {
                    tag_set(tag);
                    return;
               }

     return;
}

/** Go to the last tag
  *\param cmd uicb_t type unused
*/
void
uicb_tag_last(uicb_t cmd)
{
     (void)cmd;
     screen_get_sel();

     tag_set(conf.ntag[selscreen]);

     return;
}

/** Keep that tag the last one
  *\param cmd uicb_t type unused
*/
void
uicb_tag_stay_last(uicb_t cmd)
{
     (void)cmd;

     screen_get_sel();

     if(tags[selscreen][seltag[selscreen]].stay_last)
          tags[selscreen][seltag[selscreen]].stay_last = False;

     else
     {
          int i;
          remove_old_last_tag(selscreen);
          
          for(i = seltag[selscreen]; i <= conf.ntag[selscreen]; i++)
          {
               tag_swap(selscreen, seltag[selscreen], seltag[selscreen] + 1);
          }

          tag_set(conf.ntag[selscreen]);
          tags[selscreen][seltag[selscreen]].stay_last = True;
          arrange(selscreen, True);
     }


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
     (void)cmd;
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
     (void)cmd;

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
     (void)cmd;

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

     (void)cmd;

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

/** Add an additional tag to the current tag
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

/** Add an additional tag to the current tag
  *\param cmd uicb_t
 */
void
uicb_tag_toggle_additional(uicb_t cmd)
{
     screen_get_sel();

     tag_additional(selscreen, seltag[selscreen], atoi(cmd));

     return;
}

/** Swap 2 tags
  *\param s  Screen
  *\param t1 Tag 1
  *\param t2 Tag 2
*/
void
tag_swap(int s, int t1, int t2)
{
     Client *c;
     Tag t;

     if(t1 > conf.ntag[s] || t1 < 1
               || t2 > conf.ntag[s] || t2 < 1 || t1 == t2)
          return;

     t = tags[s][t1];
     tags[s][t1] = tags[s][t2];
     tags[s][t2] = t;

     for(c = clients; c; c = c->next)
     {
          if(c->screen == s && c->tag == (uint)t1)
               c->tag = t2;
          else if(c->screen == s && c->tag == (uint)t2)
               c->tag = t1;
     }

     infobar_update_taglist(s);
     tag_set(t2);

     return;
}

/** Swap current tag with a specified tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap(uicb_t cmd)
{
     screen_get_sel();

     tag_swap(selscreen, seltag[selscreen], atoi(cmd));

     return;
}

/** Swap current tag with next tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap_next(uicb_t cmd)
{
     (void)cmd;
     screen_get_sel();

     /* Check if the next one does have the stay_last bool */
     if(!tags[selscreen][conf.ntag[selscreen]].stay_last)
     {
          tag_swap(selscreen, seltag[selscreen], seltag[selscreen] + 1);
     }
     else
     {
          warnx("The next tag is set to always stay the last one");
     }

     return;
}

/** Swap current tag with previous tag
  *\param cmd uicb_t type
*/
void
uicb_tag_swap_previous(uicb_t cmd)
{
     (void)cmd;
     screen_get_sel();

     tag_swap(selscreen, seltag[selscreen], seltag[selscreen] - 1);

     return;
}

/** Adding a tag
  *\param s Screen number
  *\param name New tag name
*/
void
tag_new(int s, char *name)
{
     char * displayedName;
     int goToTag;

     if(conf.ntag[s] + 1 > MAXTAG)
     {
          warnx("Too many tag: Can't create new tag");

          return;
     }

     ++conf.ntag[s];

     /* TODO: memleak here */
     if(!name || strlen(name) == 0)
     {
         if(conf.tagnamecount)
         {
             /* displayedName = zmalloc(2); */
             xasprintf(&displayedName, "[%d]", conf.ntag[s]);
         }
         else
             displayedName = conf.default_tag.name;
     }
     else
         displayedName = xstrdup(name);


     Tag t = { displayedName, NULL, 0, 0,
               conf.default_tag.mwfact, conf.default_tag.nmaster,
               False, conf.default_tag.resizehint, False, False,
               conf.default_tag.barpos, conf.default_tag.barpos, conf.default_tag.layout,
               0, NULL, 0, False };

     tags[s][conf.ntag[s]] = t;

     /* For stay_last_tag */
     if(tags[s][conf.ntag[s]-1].stay_last)
     {
          tag_swap(s, conf.ntag[s], conf.ntag[s]-1);
          goToTag = conf.ntag[s]-1;
     }
     else
          goToTag = conf.ntag[s];

     infobar_update_taglist(s);
     infobar_draw(s);
     tag_set(goToTag);

     return;
}

/** Adding a tag
  *\param cmd uicb_t type
*/
void
uicb_tag_new(uicb_t cmd)
{
     screen_get_sel();

     tag_new(selscreen, (char*)cmd);

     return;
}

/** Delete a tag
  *\param s Screen number
  *\param tag Tag number
*/
void
tag_delete(int s, int tag)
{
     Tag t;
     Client *c;
     size_t i;

     memset(&t, 0, sizeof(t));

     if(tag < 0 || tag > conf.ntag[s] || conf.ntag[s] == 1)
          return;

     for(c = clients; c; c = c->next)
          if(c->screen == s && c->tag == (uint)tag)
          {
               warnx("Client(s) present in this tag, can't delete it");

               return;
          }

     --conf.ntag[s];

     tags[s][tag] = t;
     infobar[s].tags[tag] = NULL;

     for(i = tag; i < (size_t)conf.ntag[s] + 1; ++i)
     {
          /* Set clients tag because of shift */
          for(c = clients; c; c = c->next)
               if(c->screen == s && c->tag == i + 1)
                    c->tag = i;

          /* shift */
          tags[s][i] = tags[s][i + 1];
     }

     infobar[s].need_update = True;
     infobar_update_taglist(s);
     infobar_draw(s);

     if(tag == seltag[s])
          tag_set(tag <= conf.ntag[s] ? tag : conf.ntag[s]);

     return;
}

/** Delete a tag
  *\param cmd uicb_t type
*/
void
uicb_tag_del(uicb_t cmd)
{
     int n;

     screen_get_sel();

     if(cmd == NULL || !(n = atoi(cmd)))
          n = seltag[selscreen];

     tag_delete(selscreen, n);

     return;
}

/** Rename the selected tag
  *\param cmd uicb_t type
*/
void
uicb_tag_rename(uicb_t cmd)
{
     screen_get_sel();
     char *str;
     size_t len;

     if(!cmd || !strlen(cmd))
          return;

     str = tags[selscreen][seltag[selscreen]].name;
     len = strlen(str);

     /* TODO: if strlen(cmd) > len, the tag name
      * will be truncated...
      * We can't do a realloc because if the pointer change
      * free() on paser will segfault.on free_conf()...
      */
     strncpy(str, cmd, len);

     infobar_update_taglist(selscreen);
     infobar_draw(selscreen);

     return;
}

/**
  *\param selscreen int
*/
void
remove_old_last_tag(int selscreen)
{
     int i;
     for(i = 0; i <= conf.ntag[selscreen]; i++)
     {
          if(tags[selscreen][i].stay_last)
          {
              tags[selscreen][i].stay_last = False;
              break;
          }
     }

     return;
}
