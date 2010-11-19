/*
 * Copyright (c) 2010 Philippe Pepiot <phil@philpep.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "parse.h"

extern TAILQ_HEAD(, conf_sec) config;

static const struct opt_type opt_type_null = { 0, 0, False, NULL };

static struct opt_type
string_to_opt(char *s)
{
     struct opt_type ret = opt_type_null;

     if (!s || !strlen(s))
          return ret;

     ret.num = strtol(s, (char**)NULL, 10);
     ret.fnum = strtod(s, NULL);

     if (!strcmp(s, "true") || !strcmp(s, "True") ||
               !strcmp(s, "TRUE") || !strcmp(s, "1"))
          ret.bool = True;
     else
          ret.bool = False;

     ret.str = s;

     return ret;
}


void
print_unused(struct conf_sec *sec)
{
     struct conf_sec *s;
     struct conf_opt *o;

     if (!sec)
     {
          TAILQ_FOREACH(s, &config, entry)
               print_unused(s);
          return;
     }

     SLIST_FOREACH(o, &sec->optlist, entry)
          if (o->used == False)
               warnx("%s:%d, unused param %s",
                         o->filename, o->line, o->name);

     TAILQ_FOREACH(s, &sec->sub, entry)
          if (!TAILQ_EMPTY(&s->sub))
               print_unused(s);
}

struct conf_sec **
fetch_section(struct conf_sec *s, char *name)
{
     struct conf_sec **ret;
     struct conf_sec *sec;
     size_t i = 0;

     if (!name)
          return NULL;

     if (!s) {
          ret = xcalloc(2, sizeof(struct conf_sec *));
          TAILQ_FOREACH(sec, &config, entry)
               if (!strcmp(sec->name, name)) {
                    ret[0] = sec;
                    ret[1] = NULL;
                    break;
               }
     }
     else {
          ret = xcalloc(s->nsub+1, sizeof(struct conf_sec *));
          TAILQ_FOREACH(sec, &s->sub, entry) {
               if (!strcmp(sec->name, name) && i < s->nsub)
                    ret[i++] = sec;
          }
          ret[i] = NULL;
     }
     return ret;
}

struct conf_sec *
fetch_section_first(struct conf_sec *s, char *name)
{
     struct conf_sec *sec, *ret = NULL;

     if (!name)
          return NULL;

     if (!s)
     {
          TAILQ_FOREACH(sec, &config, entry)
               if(sec->name && !strcmp(sec->name, name)) {
                   ret = sec;
                   break;
               }
     }
     else
     {
         TAILQ_FOREACH(sec, &s->sub, entry)
              if (sec->name && !strcmp(sec->name, name)) {
                  ret = sec;
                  break;
              }
     }

     return ret;
}

size_t
fetch_section_count(struct conf_sec **s)
{
     size_t ret;
     for (ret = 0; s[ret]; ret++);
     return ret;
}

struct opt_type *
fetch_opt(struct conf_sec *s, char *dfl, char *name)
{
     struct conf_opt *o;
     struct opt_type *ret;
     size_t i = 0;

     if (!name)
          return NULL;

     ret = xcalloc(10, sizeof(struct opt_type));

     if (s) {
          SLIST_FOREACH(o, &s->optlist, entry)
               if (!strcmp(o->name, name)) {
                    while (o->val[i]) {
                         o->used = True;
                         ret[i] = string_to_opt(o->val[i]);
                         i++;
                    }
                    ret[i] = opt_type_null;
                    return ret;
               }
     }

     ret[0] = string_to_opt(dfl);
     ret[1] = opt_type_null;

     return ret;
}

struct opt_type
fetch_opt_first(struct conf_sec *s, char *dfl, char *name)
{
     struct conf_opt *o;

     if (!name)
          return opt_type_null;
     else if (s)
          SLIST_FOREACH(o, &s->optlist, entry)
               if (!strcmp(o->name, name)) {
                    o->used = True;
                    return string_to_opt(o->val[0]);
               }
     return string_to_opt(dfl);
}

size_t
fetch_opt_count(struct opt_type *o)
{
     size_t ret;
     for(ret = 0; o[ret].str; ret++);
     return ret;
}


