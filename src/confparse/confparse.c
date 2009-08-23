/*
*      confparse.c
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
file_to_str(char *path)
{
     char *buf, *ret, *p;
     int fd, i;
     struct stat st;

     if(!path || !(fd = open(path, O_RDONLY)))
          return NULL;

     /* Get the file size */
     stat(path, &st);

     /* Bufferize file */
     if((buf = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, SEEK_SET)) == (char*) -1)
          return NULL;

     /* Copy buffer in return value */
     ret = _strdup(buf);

     /* Unmap buffer, thanks linkdd. */
     munmap(buf, st.st_size);
     close(fd);

     /* Erase comment line from return value */
     for(i = 0; (p = strchr(erase_delim_content(ret + i), COMMENT_CHAR));)
          for(i = st.st_size - strlen(p); ret[i] && ret[i] != '\n'; ret[i++] = ' ');

     fprintf(stderr, "WMFS Configuration info: '%s' read.\n", path);

     return ret;
}

char*
get_sec(char *src, char *name)
{
     char *ret, *p;
     char **sec;

     if(!src)
          return NULL;

     if(!name)
          return src;

     sec = secname(name);

     ret = _strdup(src);

     if((p = strstr(erase_delim_content(src), sec[SecStart])))
     {
          ret += strlen(ret) - strlen(p) + strlen(sec[SecStart]) + 1;

          if((p = strstr(erase_delim_content(ret), sec[SecEnd])))
               *(ret + (strlen(ret) - strlen(p))) = '\0';
          else
               ret = NULL;
     }
     else
          ret = NULL;

     free_secname(sec);

     return ret;
}

char*
get_nsec(char *src, char *name, int n)
{
     int i;
     char *ret, *buf, **sec;
     char *buf2;

     if(!src)
          return NULL;

     if(!name)
          return src;

     if(!n)
          return get_sec(src, name);

     sec = secname(name);

     buf = erase_delim_content(src);
     buf2 = erase_sec_content(buf);

     for(i = 0; i < n && (buf = strstr(buf, sec[SecStart])); ++i, buf += strlen(sec[SecStart]));

     ret = get_sec(src + strlen(src) - strlen(buf), name);

     free_secname(sec);

     return ret;
}

int
get_size_sec(char *src, char *name)
{
     int ret;
     char **sec, *buf;

     if(!src || !name)
          return 0;

     sec = secname(name);

     buf = erase_sec_content(src);

     for(ret = 0; (buf = strstr(buf, sec[SecStart])); ++ret, buf += strlen(sec[SecStart]));

     free_secname(sec);

     return ret;
}

opt_type
get_opt(char *src, char *def, char *name)
{
     int i;
     char *p = NULL, *p2 = NULL;
     opt_type ret = null_opt_type;

     if(!src || !name)
          return (def) ? str_to_opt(def) : null_opt_type;

     if((p = opt_srch(erase_sec_content(src), name)))
     {
          for(i = 0; p[i] && p[i] != '\n'; ++i);
          p[i] = '\0';

          p2 = _strdup(p + strlen(name));

          if((p = strchr(p, '=')) && !is_in_delimiter(p, 0))
          {
               for(i = 0; p2[i] && p2[i] != '='; ++i);
               p2[i] = '\0';

               /* Check if there is anything else that spaces
                * between option name and '=' */
               for(i = 0; i < strlen(p2); ++i)
                    if(p2[i] != ' ')
                         return str_to_opt(def);

               ret = str_to_opt(clean_value(++p));
          }
     }
     else
          ret = str_to_opt(def);

     return ret;
}

/* option = {val1, val2, val3} */
opt_type*
get_list_opt(char *src, char *def, char *name, int *n)
{
     int i, j;
     char *p, *p2;
     opt_type *ret;

     if(!src || !name)
          return NULL;

     *n = 0;

     if(!(p = get_opt(src, def, name).str))
          return NULL;

     for(i = 0; p[i] && (p[i] != LIST_DEL_E || is_in_delimiter(p, i)); ++i);
     p[i + 1] = '\0';

     /* Syntax of list {val1, val2, ..., valx} */
     if(*p != LIST_DEL_S || *(p + strlen(p) - 1) != LIST_DEL_E)
          return NULL;

     /* Erase ( ) */
     ++p;
     *(p + strlen(p) - 1) = '\0';

     /* > 1 value in list */
     if(strchr(p, ','))
     {
          /* Count ',' */
          for(i = 0, *n = 1; i < strlen(p); ++i)
               if(p[i] == ',' && !is_in_delimiter(p, i))
                    ++(*n);

          ret = emalloc(*n, sizeof(opt_type));

          p2 = _strdup(p);

          /* Set all value in return array */
          for(i = j = 0; i < *n; ++i, p2 += ++j)
          {
               for(j = 0; j < strlen(p2) && (p2[j] != ',' || is_in_delimiter(p2, j)); ++j);
               p2[j] = '\0';

               ret[i] = str_to_opt(clean_value(p2));
          }
     }
     else
     {
          ret = emalloc((*n = 1), sizeof(opt_type));
          *ret = str_to_opt(clean_value(p));
     }

     return ret;
}
