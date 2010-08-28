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
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <err.h>

#include "parse.h"

extern char *__progname;

enum keyword_t { SEC_START, SEC_END, INCLUDE, WORD, EQUAL, LIST_START, LIST_END, NONE };

static struct {
     const char *name;
     enum keyword_t type;
} kw_t_name[] = {
     {"SEC_START", SEC_START},
     {"SEC_END", SEC_END},
     {"INCLUDE", INCLUDE},
     {"WORD", WORD},
     {"EQUAL", EQUAL},
     {"LIST_START", LIST_START},
     {"LIST_END", LIST_END},
     {"NONE", NONE},
};

struct files {
     char *name;
     struct files *parent;
};

struct keyword {
     enum keyword_t type;
     /* if WORD */
     int line;
     struct files *file;
     char *name;
     struct keyword *next;
};

struct state {
     bool_t quote;
     bool_t comment;
     char quote_char;
};

/* TO REMOVE (use a identifier for config and fallback XDG in api functions) */
TAILQ_HEAD(, conf_sec) config;

static struct keyword *
push_keyword(struct keyword *tail, enum keyword_t type, char *buf, size_t *offset, struct files *file, int line)
{
     struct keyword *kw;
     int i = 0;

     if (type == WORD && *offset == 0)
          return tail;

     kw = xcalloc(1, sizeof(*kw));
     kw->type = type;
     kw->line = line;
     kw->file = file;
     kw->next = NULL;

     if (*offset != 0) {
          buf[*offset] = '\0';
          if (!strcmp(buf, INCLUDE_CMD))
               type = kw->type = INCLUDE;
          else
               kw->name = strdup(buf);
          *offset = 0;
     }
     else
          kw->name = NULL;

     if (tail)
          tail->next = kw;

#ifdef DEBUG
     for (i = 0; kw_t_name[i].type != NONE; i++) {
          if (kw_t_name[i].type == kw->type) {
               warnx("%s %s %s:%d\n", kw_t_name[i].name,
                         (kw->name) ? kw->name : "",
                         kw->file->name, kw->line);
          }
     }
#endif

     return kw;
}

#define PUSH_KEYWORD(type) tail = push_keyword(tail, type, bufname, &j, file, line)
static struct keyword *
parse_keywords(const char *filename)
{
     int fd;
     struct stat st;
     char *buf;

     struct keyword *head = NULL;
     struct keyword *tail = NULL;
     struct files *file;
     enum keyword_t type; /* keyword type to push */
     struct state s = { False, False, '\0'};
     char *bufname;
     char path[PATH_MAX];
     size_t i, j;
     int line;


     if ((fd = open(filename, O_RDONLY)) == -1 || stat(filename, &st) == -1) {
          warn("%s", filename);
          return NULL;
     }

     buf = (char *)mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, SEEK_SET);

     if (buf == (char*)MAP_FAILED) {
          warn("%s", filename);
          return NULL;
     }

     if (!realpath(filename, path)) {
          warn("%s", filename);
          return NULL;
     }

     file = xcalloc(1, sizeof(*file));
     file->name = strdup(path);
     file->parent = NULL;

     bufname = xcalloc(1, sizeof(*bufname) * BUFSIZ);


     for(i = 0, j = 0, line = 1; i < (size_t)st.st_size; i++) {

          if (tail && !head)
               head = tail;

          if (buf[i] == '\n' && s.comment == True) {
               line++;
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
               /* end of quotted string */
               PUSH_KEYWORD(WORD);
               s.quote = False;
               continue;
          }

          if ((buf[i] == '"' || buf[i] == '\'') &&
                    s.quote == False)
          {
               PUSH_KEYWORD(WORD);
               /* begin quotted string */
               s.quote_char = buf[i];
               s.quote = True;
               continue;
          }

          if (buf[i] == '[' && s.quote == False) {
               PUSH_KEYWORD(WORD);
               if (buf[i+1] == '/') {
                    i +=2;
                    type = SEC_END;
               }
               else {
                    i++;
                    type = SEC_START;
               }

               while (buf[i] != ']') {
                    if (i >= (size_t)st.st_size)
                    {
                         /* TODO ERREUR */
                    }
                    bufname[j++] = buf[i++];
               }
               PUSH_KEYWORD(type);
               continue;
          }

          if (buf[i] == '{' && s.quote == False) {
               PUSH_KEYWORD(WORD);
               PUSH_KEYWORD(LIST_START);
               continue;
          }

          if (buf[i] == '}' && s.quote == False) {
               PUSH_KEYWORD(WORD);
               PUSH_KEYWORD(LIST_END);
               continue;
          }

          if (buf[i] == ',' && s.quote == False) {
               PUSH_KEYWORD(WORD);
               continue;
          }

          if (buf[i] == '=' && s.quote == False) {
               PUSH_KEYWORD(WORD);
               PUSH_KEYWORD(EQUAL);
               continue;
          }

          if (strchr("\t\n ", buf[i]) && s.quote == False) {
               PUSH_KEYWORD(WORD);

               if (buf[i] == '\n')
                    line++;

               continue;
          }

          bufname[j++] = buf[i];
     }
     munmap(buf, st.st_size);
     warnx("%s read", file->name);
     return head;
}

static void
syntax(struct keyword *kw, const char *fmt, ...)
{
     va_list args;

     fprintf(stderr, "%s: %s:%d", __progname, kw->file->name, kw->line);
     if (kw->name)
          fprintf(stderr, ", near '%s'", kw->name);
     fprintf(stderr, ": ");

     va_start(args, fmt);
     vfprintf(stderr, fmt, args);
     va_end(args);

     fprintf(stderr, "\n");

     exit(EXIT_FAILURE);
}

static struct keyword *
include(struct keyword *head)
{
     struct keyword *kw;
     struct keyword *tail;
     struct files *file;

     head = head->next;

     if (!head || head->type != WORD)
          syntax(head, "missing filename to include");

     if (!(kw = parse_keywords(head->name))) {
          warnx("no config fond in include file %s", head->name);
          return head->next;
     }

     kw->file->parent = head->file;
     head = head->next;

     /* detect circular include */
     for (file = kw->file->parent; file != NULL; file = file->parent)
     {
          if (!strcmp(file->name, kw->file->name))
               syntax(kw, "circular include of %s", kw->file->name);
     }

     if (kw) {
          for (tail = kw; tail->next; tail = tail->next);
          tail->next = head;
     }

     return kw;
}

static struct conf_opt *
get_option(struct keyword **head)
{
     struct conf_opt *o;
     size_t j = 0;
     struct keyword *kw = *head;

     o = xcalloc(1, sizeof(*o));
     o->name = kw->name;
     o->used = False;
     o->line = kw->line;
     o->filename = kw->file->name;

     kw = kw->next;

     if (kw->type != EQUAL)
          syntax(kw, "missing '=' here");

     kw = kw->next;

     if (!kw)
          syntax(kw, "missing value");


     switch (kw->type) {
          case INCLUDE:
               kw = include(kw);
               break;
          case WORD:
               o->val[0] = kw->name;
               o->val[1] = NULL;
               kw = kw->next;
               break;
          case LIST_START:
               kw = kw->next;
               while (kw && kw->type != LIST_END) {
                    switch (kw->type) {
                         case WORD:
                              if (j > 9)
                                   syntax(kw, "too much values in list");
                              o->val[j++] = kw->name;
                              kw = kw->next;
                              break;
                         case INCLUDE:
                              kw = include(kw);
                              break;
                         default:
                              syntax(kw, "declaration into a list");
                              break;
                    }
               }

               if (!kw)
                    syntax(kw, "list unclosed");

               kw = kw->next;
               break;
          default:
               syntax(kw, "missing value");
               break;
     }

     *head = kw;
     return o;
}

static struct conf_sec *
get_section(struct keyword **head)
{
     struct conf_sec *s;
     struct conf_opt *o;
     struct conf_sec *sub;
     struct keyword *kw = *head;

     s = xcalloc(1, sizeof(*s));
     s->name = kw->name;
     TAILQ_INIT(&s->sub);
     SLIST_INIT(&s->optlist);

     kw = kw->next;

     while (kw && kw->type != SEC_END) {
          switch (kw->type) {
               case INCLUDE:
                    kw = include(kw);
                    break;
               case SEC_START:
                    sub = get_section(&kw);
                    TAILQ_INSERT_TAIL(&s->sub, sub, entry);
                    s->nsub++;
                    break;
               case WORD:
                    o = get_option(&kw);
                    SLIST_INSERT_HEAD(&s->optlist, o, entry);
                    s->nopt++;
                    break;
               default:
                    syntax(kw, "syntax error");
                    break;
          }
     }

     if (!kw || strcmp(kw->name, s->name))
          syntax(kw, "missing end section %s", s->name);

     kw = kw->next;
     *head = kw;

     return s;
}

int
get_conf(const char *filename)
{
     struct conf_sec *s;
     struct keyword *head, *kw;

     kw = head = parse_keywords(filename);
     if (!head)
          return -1; /* TODO ERREUR */

     TAILQ_INIT(&config);

     while (kw) {
          switch (kw->type) {
               case INCLUDE:
                    kw = include(kw);
                    break;
               case SEC_START:
                    s = get_section(&kw);
                    TAILQ_INSERT_TAIL(&config, s, entry);
                    break;
               default:
                    syntax(kw, "out of any section");
                    break;
          }
     }
     return 0;
}


void *
xcalloc(size_t nmemb, size_t size)
{
     void *ret;

     if (!(ret = calloc(nmemb, size)))
          warn("calloc");

     return ret;
}
