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

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>

#include "../wmfs.h"


static void get_keyword(const char *buf, size_t n);
static void pop_keyword(void);
static void pop_stack(void);
static struct conf_sec *get_section(void);
static struct conf_opt *get_option(void);
static struct opt_type string_to_opt(char *);
#ifdef DEBUG
static void print_kw_tree(void);
static char * get_kw_name(enum conf_type);
#endif

static TAILQ_HEAD(, conf_keyword) keywords;
static TAILQ_HEAD(, conf_stack) stack;
static SLIST_HEAD(, conf_sec) config;
static struct conf_keyword *curk; /* current keyword */
static struct conf_stack *curw; /* current word */
static const struct opt_type opt_type_null = { 0, 0, False, NULL };

static struct {
     const char *name;
     int line;
} file = { NULL, 1 };

static void
get_keyword(const char *buf, size_t n)
{
     struct conf_keyword *kw;
     size_t j, i;
     struct conf_state s = { False, False, '\0' };
     struct conf_stack *e;

     TAILQ_INIT(&stack);
     TAILQ_INIT(&keywords);
     kw = emalloc(1, sizeof(*kw));
     e = emalloc(1, sizeof(*e));

     for(i = 0, j = 0; i < n; i++)
     {
          if (buf[i] == '\n' && s.comment == True) {
               file.line++;
               s.comment = False;
               continue;
          }

          if (buf[i] == '#' && s.quote == False) {
               s.comment = True;
               continue;
          }

          if (s.comment == True)
               continue;

          if (buf[i] == s.quote_char && s.quote == True) {
               NEW_WORD();
               s.quote = False;
               continue;
          }

          if ((buf[i] == '"' || buf[i] == '\'') &&
                    s.quote == False)
          {
               s.quote_char = buf[i];
               s.quote = True;
               continue;
          }

          if (buf[i] == '[' && s.quote == False) {
               NEW_WORD();
               TOKEN((buf[i+1] == '/') ? SEC_END : SEC_START);
               if (buf[i+1] == '/')
                    i++;
               continue;
          }

          if (buf[i] == ']' && s.quote == False) {
               NEW_WORD();
               continue;
          }

          if (buf[i] == '{' && s.quote == False) {
               NEW_WORD();
               TOKEN(LIST_START);
               continue;
          }

          if (buf[i] == '}' && s.quote == False) {
               NEW_WORD();
               TOKEN(LIST_END);
               continue;
          }

          if (buf[i] == ',' && s.quote == False) {
               NEW_WORD();
               continue;
          }

          if (buf[i] == '=' && s.quote == False) {
               NEW_WORD();
               TOKEN(EQUAL);
               continue;
          }

          if (strchr("\t\n ", buf[i]) && s.quote == False) {
               NEW_WORD();
               if (buf[i] == '\n')
                    file.line++;
               continue;
          }

          e->name[j++] = buf[i];
     }
}

#ifdef DEBUG
static void
print_kw_tree(void)
{
     struct conf_keyword *k;
     struct conf_stack *s;

     s = TAILQ_FIRST(&stack);

     TAILQ_FOREACH(k, &keywords, entry)
          printf("%s ", get_kw_name(k->type));
     printf("\n");
}

static char *
get_kw_name(enum conf_type type)
{
     switch (type) {
          case SEC_START:
               return ("SEC_START");
               break;
          case SEC_END:
               return ("SEC_END");
               break;
          case WORD:
               return ("WORD");
               break;
          case LIST_START:
               return ("LIST_START ");
               break;
          case LIST_END:
               return ("LIST_END ");
               break;
          case EQUAL:
               return ("EQUAL ");
               break;
          default:
               return ("NONE ");
               break;
     }
}
#endif

int
get_conf(const char *name)
{
     int fd;
     struct stat st;
     char *buf;
     struct conf_sec *s;

     if (!name)
          return (-1);

     if ((fd = open(name, O_RDONLY)) == -1 ||
               stat(name, &st) == -1)
     {
          warn("%s", name);
          return (-1);
     }

     buf = (char*)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, SEEK_SET);

     if (buf == (char*) MAP_FAILED)
          return -1;

     get_keyword(buf, st.st_size);

     munmap(buf, st.st_size);
     close(fd);
     warnx("%s read", name);

     file.name = name;

     curk = TAILQ_FIRST(&keywords);
     curw = TAILQ_FIRST(&stack);

     SLIST_INIT(&config);

     while (!TAILQ_EMPTY(&keywords)) {
          switch (curk->type) {
               case SEC_START:
                    s = get_section();
                    SLIST_INSERT_HEAD(&config, s, entry);
                    break;
               default:
                    errx(1, "%s:%d: near '%s', config out of any section",
                              file.name, curw->line, curw->name);
                    break;
          }
     }
     return 0;
}

static struct conf_sec *
get_section(void)
{
     struct conf_sec *s;
     struct conf_opt *o;
     struct conf_sec *sub;

     s = emalloc(1, sizeof(*s));
     s->name = strdup(curw->name);
     pop_stack();
     pop_keyword();

     if (curk->type != WORD)
          errx(1, "%s:%d: near '%s', missing section name",
                    file.name, curw->line, curw->name);
     pop_keyword();

     while (curk->type != SEC_END) {
          switch (curk->type) {
               case WORD:
                    o = get_option();
                    SLIST_INSERT_HEAD(&s->optlist, o, entry);
                    s->nopt++;
                    break;
               case SEC_START:
                    sub = get_section();
                    SLIST_INSERT_HEAD(&s->sub, sub, entry);
                    s->nsub++;
               case SEC_END:
                    break;
               default:
                    errx(1, "%s:%d: near '%s', syntax error",
                              file.name, curw->line, curw->name);
                    break;
          }
     }
     pop_keyword();

     if (curk->type != WORD)
          errx(1, "%s:%d: near '%s', missing end-section name",
                    file.name, curw->line, curw->name);

     if (strcmp(curw->name, s->name))
          errx(1, "%s:%d: near '%s', non-closed section '%s'",
                    file.name, curw->line, curw->name, s->name);

     pop_stack();
     pop_keyword();
     return s;
}


static struct conf_opt *
get_option(void)
{
     struct conf_opt *o;
     size_t j = 0;

     o = emalloc(1, sizeof(*o));
     o->name = strdup(curw->name);
     o->used = False;
     o->line = curw->line;
     pop_stack();
     pop_keyword();

     if (curk->type != EQUAL)
          errx(1, "%s:%d: near '%s', missing '=' here",
                    file.name, curw->line, curw->name);

     pop_keyword();

     switch (curk->type) {
          case WORD:
               o->val[0] = strdup(curw->name);
               o->val[1] = NULL;
               pop_stack();
               break;
          case LIST_START:
               pop_keyword();
               while (curk->type != LIST_END) {
                    if (curk->type != WORD)
                         errx(1, "%s:%d: near '%s', declaration into a list",
                                   file.name, curw->line, curw->name);
                    o->val[j++] = strdup(curw->name);
                    pop_stack();
                    pop_keyword();
               }
               o->val[j] = NULL;
               break;
          default:
               errx(1, "%s:%d: near '%s', syntax error",
                         file.name, curw->line, curw->name);
               break;
     }
     pop_keyword();
     return o;
}


static void
pop_keyword(void)
{
     TAILQ_REMOVE(&keywords, curk, entry);
#ifdef DEBUG
     warnx("%s", get_kw_name(curk->type));
#endif
     free(curk);

     curk = TAILQ_FIRST(&keywords);
}

static void
pop_stack(void)
{
     TAILQ_REMOVE(&stack, curw, entry);
#ifdef DEBUG
     warnx("%s", curw->name);
#endif
     free(curw);

     curw = TAILQ_FIRST(&stack);
}

void
print_unused(struct conf_sec *s)
{
     struct conf_sec *sec;
     struct conf_opt *o;

     if (!s)
     {
          SLIST_FOREACH(sec, &config, entry)
               print_unused(sec);
          return;
     }

     SLIST_FOREACH(o, &s->optlist, entry)
          if (o->used == False)
               warnx("%s:%d, unused param %s",
                         file.name, o->line, o->name);

     SLIST_FOREACH(sec, &s->sub, entry)
          if (!SLIST_EMPTY(&sec->sub))
               print_unused(sec);
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
          ret = emalloc(2, sizeof(struct conf_sec *));
          SLIST_FOREACH(sec, &config, entry)
               if (!strcmp(sec->name, name)) {
                    ret[0] = sec;
                    ret[1] = NULL;
                    break;
               }
     }
     else {
          ret = emalloc(s->nsub+1, sizeof(struct conf_sec *));
          SLIST_FOREACH(sec, &s->sub, entry) {
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
     struct conf_sec *sec;

     if (!name)
          return NULL;

     if (!s)
          SLIST_FOREACH(sec, &config, entry)
               if (!strcmp(sec->name, name))
                    return sec;

     SLIST_FOREACH(sec, &s->sub, entry)
          if (!strcmp(sec->name, name))
               return sec;

     return NULL;
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

     ret = emalloc(10, sizeof(struct opt_type));

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

static struct opt_type
string_to_opt(char *s)
{
     struct opt_type ret = opt_type_null;

     if (!s || !strlen(s))
          return ret;

     ret.num = atoi(s);
     sscanf(s, "%f", &ret.fnum);

     if (!strcmp(s, "true") || !strcmp(s, "True") ||
               !strcmp(s, "TRUE") || !strcmp(s, "1"))
          ret.bool = True;
     else
          ret.bool = False;

     ret.str = s;

     return ret;
}

