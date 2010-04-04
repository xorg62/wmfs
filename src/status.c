/*
*      status.c
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

/** Check rectangles blocks in str and return properties
  * --> \b[x;y;width;height;#color]\
  *\param r StatusRec pointer, rectangles properties
  *\param str String
  *\return n Length of r
  */
int
statustext_rectangle(StatusRec *r, char *str)
{
     char as;
     int n, i, j, k;

     for(i = j = n = 0; i < strlen(str); ++i, ++j)
          if(sscanf(&str[i], "\\b[%d;%d;%d;%d;#%x]%c", &r[n].x, &r[n].y, &r[n].w, &r[n].h, &r[n].color, &as) == 6
                    && as == '\\')
               for(++n, ++i, --j; str[i] != as || str[i - 1] != ']'; ++i);
          else if(j != i)
               str[j] = str[i];

     for(k = j; k < i; str[k++] = 0);

     return n;
}

/** Check text blocks in str and return properties
  * --> \s[x;y;#color;text]\
  *\param s StatusText pointer, text properties
  *\param str String
  *\return n Length of s
  */
int
statustext_text(StatusText *s, char *str)
{
     char as;
     int n, i, j, k;

     for(i = j = n = 0; i < strlen(str); ++i, ++j)
          if(sscanf(&str[i], "\\s[%d;%d;%7[^;];%512[^]]]%c", &s[n].x, &s[n].y, s[n].color, s[n].text, &as) == 5
                    && as == '\\')
               for(++n, ++i, --j; str[i] != as || str[i - 1] != ']'; ++i);
          else if(j != i)
               str[j] = str[i];

     for(k = j; k < i; str[k++] = 0);

     return n;
}

/** Draw normal text and colored normal text
  * --> \#color\ text in color
  *\param sc Screen
  *\param str String
  */
void
statustext_normal(int sc, char *str)
{
     char strwc[MAXSTATUS] = { 0 };
     char buf[MAXSTATUS] = { 0 };
     char col[8] = { 0 };
     int n, i, j, k;

     for(i = j = n = 0; i < strlen(str); ++i, ++j)
          if(str[i] == '\\' && str[i + 1] == '#' && str[i + 8] == '\\')
          {
               ++n;
               i += 8;
               --j;
          }
          else
               strwc[j] = str[i];

     /* Draw normal text without any blocks */
     draw_text(infobar[sc].bar->dr, (sgeo[sc].width - SHADH) - textw(strwc),
               FHINFOBAR, infobar[sc].bar->fg, 0, strwc);

     if(n)
     {
          strcpy(buf, strwc);

          for(i = k = 0; i < strlen(str); ++i, ++k)
               if(str[i] == '\\' && str[i + 1] == '#' && str[i + 8] == '\\')
               {
                    /* Store current color in col[] */
                    for(j = 0, ++i; str[i] != '\\'; col[j++] = str[i++]);

                    /* Draw a rectangle with the bar color to draw the text properly */
                    draw_rectangle(infobar[sc].bar->dr, (sgeo[sc].width - SHADH) - textw(&buf[k]),
                                   0, INFOBARH - (sgeo[sc].width - SHADH) - textw(&buf[k]),
                                   INFOBARH, conf.colors.bar);

                    /* Draw text with its color */
                    draw_text(infobar[sc].bar->dr, (sgeo[sc].width - SHADH) - textw(&buf[k]),
                              FHINFOBAR,  col, 0, &buf[k]);

                    strcpy(buf, strwc);
                    ++i;
               }
     }

     return;
}

/** Handle statustext and draw all things in infobar of specified screen
  *\param sc Screen number
  *\param str String
  */
void
statustext_handle(int sc, char *str)
{
     char *lastst;
     int i, nr, ns, len;
     StatusRec r[128];
     StatusText s[128];

     /* If the str == the current statustext, return (not needed) */
     if(!str)
          return;

     barwin_refresh_color(infobar[sc].bar);

     /* save last status text address (for free at the end) */
     lastst = infobar[sc].statustext;

     infobar[sc].statustext = _strdup(str);
     len = ((strlen(str) > MAXSTATUS) ? MAXSTATUS : strlen(str));

     /* Store rectangles, located text & images properties. */
     nr = statustext_rectangle(r, str);
     ns = statustext_text(s, str);

     /* Draw normal text (and possibly colored with \#color\ blocks) */
     statustext_normal(sc, str);

     /* Draw rectangles with stored properties. */
     for(i = 0; i < nr; ++i)
          draw_rectangle(infobar[sc].bar->dr, r[i].x, r[i].y, r[i].w, r[i].h, r[i].color);

     /* Draw located text with stored properties. */
     for(i = 0; i < ns; ++i)
          draw_text(infobar[sc].bar->dr, s[i].x, s[i].y, s[i].color, 0, s[i].text);

     barwin_refresh(infobar[sc].bar);

     free(lastst);

     return;
}
