/*
*      confparse/util.c
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

#include "confparse.h"

char*
erase_delim_content(char *buf)
{
     int i, j;
     char *str, c;

     if(!buf || !(str = _strdup(buf)))
          return NULL;

     for(i = 0; i < strlen(str); ++i)
          if((c = str[i]) == '"' || (c = str[i]) == '\'')
          {
               for(*(str + (j = i)) = ' '; str[j] && str[j] != c; str[j++] = ' ');
               str[j] = ' ';
          }

     return str;
}

/* Erase all content of all delimiters, put it
 * int str, and check if buf[p] is in an delimiter. */
Bool
is_in_delimiter(char *buf, int p)
{
     if(*(erase_delim_content(buf) + p) != buf[p])
          return True;

     return False;
}

char*
erase_sec_content(char *buf)
{
     int i, j;
     char *p, *str, *name, *ret;
     char **sec;

     if(!buf || !(str = erase_delim_content(buf)))
          return NULL;

     ret = _strdup(buf);

     for(i = 1, name = _strdup(str + i); strchr(str + i, SEC_DEL_S); ++i, name = _strdup(str + i))
     {
          for(; str[i] && str[i] != SEC_DEL_S; ++i);
          for(j = 0; str[i] && str[i] != SEC_DEL_E; name[j++] = str[i++]);
          ++name;
          name[j - 1] = '\0';

          if(*name == '/')
               continue;

          sec = secname(name);

          if((p = strstr(str + i, sec[SecEnd])))
               for(++i; i < strlen(ret) - strlen(p); ret[i++] = ' ');
          else
               break;
     }

     free_secname(sec);

     return ret;
}

/* To get the RIGHT name of an option; if option needed is
 * pwet and there is tagadapwettagada in the configuration,
 * with strstr(), the name will matchs */
char*
opt_srch(char *buf, char *opt)
{
     char *p;

     if(!buf || !opt)
          return NULL;

     if((p = strstr(erase_delim_content(buf), opt)))
          if((*(p + strlen(opt)) == ' ' || *(p + strlen(opt)) == '=')
             && (*(p - 1) == ' ' || *(p - 1) == '\n' || *(p - 1) == '\t' || !(*(p - 1))))
             return _strdup(buf + (strlen(buf) - strlen(p)));

     return NULL;
}

opt_type
str_to_opt(char *str)
{
     opt_type ret = null_opt_type;

     if(!strlen(str))
          return ret;

     /* Integer */
     ret.num = atoi(str);

     /* Float */
     sscanf(str, "%f", &ret.fnum);

     /* Boolean */
     if(strstr(str, "true") || strstr(str, "True")
        || strstr(str, "TRUE") || strstr(str, "1"))
          ret.bool = True;

     /* String */
     ret.str = _strdup(str);

     return ret;
}

char*
clean_value(char *str)
{
     int i;
     char c, *p;

     if(!str || !(p = _strdup(str)))
          return NULL;

     /* Remove useless spaces */
     for(; *p == ' '; ++p);
     for(; *(p + strlen(p) - 1) == ' '; *(p + strlen(p) - 1) = '\0');

     /* For string delimiter (" or ') */
     if(((c = *p) == '"' || (c = *p) == '\'') && strchr(p + 1, c))
     {
          for(++p, i = 0; p[i] && p[i] != c; ++i);
          p[i] = '\0';
     }

     return p;
}

char**
secname(char *name)
{
     char **ret = NULL;

     if(!name)
          return NULL;

     ret = emalloc(SecLast, sizeof(char*));

     /* Len of name + '[' + ']' + '\0' */
     ret[SecStart] = emalloc(strlen(name) + 3, sizeof(char));

     /* Len of name + '[' + '/' + ']' + '\0' */
     ret[SecEnd] = emalloc(strlen(name) + 4, sizeof(char));

     sprintf(ret[SecStart], "%c%s%c", SEC_DEL_S, name, SEC_DEL_E);
     sprintf(ret[SecEnd], "%c/%s%c", SEC_DEL_S, name, SEC_DEL_E);

     return ret;
}

void
free_secname(char **secname)
{
     if(!secname || LEN(secname) != SecLast)
          return;

     if(secname[SecStart])
          free(secname[SecStart]);
     if(secname[SecEnd])
          free(secname[SecEnd]);

     free(secname);

     return;
}
