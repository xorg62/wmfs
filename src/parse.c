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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <err.h>

#include "parse.h"

extern char *__progname;

enum keyword_t { SEC_START, SEC_END, INCLUDE, WORD, EQUAL, LIST_START, LIST_END, NONE };

#ifdef DEBUG
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
#endif

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
static struct keyword *keywords = NULL;

static struct keyword *
push_keyword(struct keyword *tail, enum keyword_t type, char *buf, size_t *offset, struct files *file, int line)
{
     struct keyword *kw;
#ifdef DEBUG
     int i = 0;
#endif

     if (type == WORD && *offset == 0)
          return tail;

     kw = zcalloc(sizeof(*kw));
     kw->type = type;
     kw->line = line;
     kw->file = file;
     kw->next = NULL;

     if (*offset != 0) {
          buf[*offset] = '\0';
          if (!strcmp(buf, INCLUDE_CMD))
               kw->type = INCLUDE;
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

static void
syntax(struct keyword *kw, const char *fmt, ...)
{
     va_list args;

     fprintf(stderr, "%s:", __progname);

     if (kw && kw->file && kw->file->name)
          fprintf(stderr, "%s:%d", kw->file->name, kw->line);

     if (kw && kw->name)
          fprintf(stderr, ", near '%s'", kw->name);
     fprintf(stderr, ": ");

     va_start(args, fmt);
     vfprintf(stderr, fmt, args);
     va_end(args);

     fprintf(stderr, "\n");
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
     bool_t error = False;


     if ((fd = open(filename, O_RDONLY)) == -1 || stat(filename, &st) == -1) {
          warn("%s", filename);
          return NULL;
     }

     if (st.st_size == 0) {
          warnx("%s: empty file", filename);
          close(fd);
          return NULL;
     }

     if (!realpath(filename, path)) {
          warn("%s", filename);
          close(fd);
          return NULL;
     }

     buf = zmalloc(st.st_size+1);

     if (read(fd, buf, st.st_size) == -1) {
          warn("%s", filename);
          free(buf);
          close(fd);
          return NULL;
     }

     buf[st.st_size] = '\0';

     file = zcalloc(sizeof(*file));
     bufname = zcalloc(sizeof(*bufname) * BUFSIZ);
     file->name = strdup(path);
     file->parent = NULL;

     for(i = 0, j = 0, line = 1; i < (size_t)st.st_size; i++) {

          if (!head && tail)
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

          if (s.quote == True && buf[i] == s.quote_char) {
               /* end of quotted string */
               PUSH_KEYWORD(WORD);
               s.quote = False;
               continue;
          }

          if (s.quote == False) {
               if ((buf[i] == '"' || buf[i] == '\'')) {
                    PUSH_KEYWORD(WORD);
                    /* begin quotted string */
                    s.quote_char = buf[i];
                    s.quote = True;
                    continue;
               }

               if (buf[i] == '[') {
                    PUSH_KEYWORD(WORD);
                    if (buf[i+1] == '/') {
                         i +=2;
                         type = SEC_END;
                    }
                    else {
                         i++;
                         type = SEC_START;
                    }

                    /* get section name */
                    while (buf[i] != ']') {

                         if (i >= ((size_t)st.st_size-1) || j >= (BUFSIZ-1)) {
                              bufname[j] = '\0';
                              syntax(NULL, "word too long in %s:%d near '%s'",
                                        file->name, line, bufname);
                              error = True;
                              break;
                         }

                         bufname[j++] = buf[i++];
                    }
                    PUSH_KEYWORD(type);
                    continue;
               }

               if (buf[i] == '{') {
                    PUSH_KEYWORD(WORD);
                    PUSH_KEYWORD(LIST_START);
                    continue;
               }

               if (buf[i] == '}') {
                    PUSH_KEYWORD(WORD);
                    PUSH_KEYWORD(LIST_END);
                    continue;
               }

               if (buf[i] == ',') {
                    PUSH_KEYWORD(WORD);
                    continue;
               }

               if (buf[i] == '=') {
                    PUSH_KEYWORD(WORD);
                    PUSH_KEYWORD(EQUAL);
                    continue;
               }

               if (strchr("\t\n ", buf[i])) {
                    PUSH_KEYWORD(WORD);

                    if (buf[i] == '\n')
                         line++;

                    continue;
               }
          } /* s.quote == False */

          if (j >= (BUFSIZ - 1)) {
               bufname[j] = '\0';
               syntax(NULL, "word too long in %s:%d near '%s'",
                         file->name, line, bufname);
               error = True;
               break;
          }

          bufname[j++] = buf[i];
     }

     free(buf);
     free(bufname);
     close(fd);
     warnx("%s read", file->name);

     return (error ? NULL: head);
}

/*
 * return NULL on failure and head->next if
 * no config found (of file doesn't exist)
 * NOTE to devs: head->name is the file to include
 */
static struct keyword *
include(struct keyword *head)
{
     struct keyword *kw;
     struct keyword *tail;
     struct files *file;
     struct passwd *user;
     char *filename = NULL;
     char *base = NULL;

     head = head->next;

     if (!head || head->type != WORD) {
          syntax(head, "missing filename to include");
          return NULL;
     }

     /* replace ~ by user directory */
     if (head->name && head->name[0] == '~') {
          if ( (user = getpwuid(getuid())) && user->pw_dir)
               xasprintf(&filename, "%s%s", user->pw_dir, head->name+1);
          else if (getenv("HOME"))
               xasprintf(&filename, "%s%s", getenv("HOME"), head->name+1);
          else /* to warning ? */
               filename = head->name;
     }
     /* relative path from parent file */
     else if (head->name && head->name[0] != '/') {
          base = strdup(head->file->name);
          xasprintf(&filename, "%s/%s", dirname(base), head->name);
          free(base);
     }
     else
          filename = head->name;

     if (!(kw = parse_keywords(filename))) {
          warnx("no config found in include file %s", head->name);

          if (filename != head->name)
               free(filename);

          return NULL;
     }

     kw->file->parent = head->file;

     /* detect circular include */
     for (file = kw->file->parent; file != NULL; file = file->parent) {
          if (!strcmp(file->name, kw->file->name)) {
               syntax(kw, "circular include of %s", kw->file->name);

               if (filename != head->name)
                    free(filename);

               return NULL;
          }
     }

     head = head->next;

     if (kw) {
          for (tail = kw; tail->next; tail = tail->next);
          tail->next = head;
     }

     return kw;
}

static void *
free_opt(struct conf_opt *o)
{
     free(o);
     return NULL;
}

static struct conf_opt *
get_option(struct keyword **head)
{
     struct conf_opt *o;
     size_t j = 0;
     struct keyword *kw = *head;

     o = zcalloc(sizeof(*o));
     o->name = kw->name;
     o->used = False;
     o->line = kw->line;
     o->filename = kw->file->name;

     kw = kw->next;

     if (kw->type != EQUAL) {
          syntax(kw, "missing '=' here");
          return free_opt(o);
     }

     kw = kw->next;

     if (!kw) {
          syntax(kw, "missing value");
          return free_opt(o);
     }


     switch (kw->type) {
          case INCLUDE:
               if (!(kw = include(kw)))
                    return free_opt(o);
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
                              if (j >= (PARSE_MAX_LIST - 1)) {
                                   syntax(kw, "too much values in list");
                                   return free_opt(o);
                              }
                              o->val[j++] = kw->name;
                              kw = kw->next;
                              break;
                         case INCLUDE:
                              if (!(kw = include(kw)))
                                   return free_opt(o);
                              break;
                         default:
                              syntax(kw, "declaration into a list");
                              return free_opt(o);
                              break;
                    }
               }

               if (!kw) {
                    syntax(kw, "list unclosed");
                    return free_opt(o);
               }

               kw = kw->next;
               break;
          default:
               syntax(kw, "missing value");
               return free_opt(o);
               break;
     }

     *head = kw;
     return o;
}

static void *
free_sec(struct conf_sec *sec)
{
     struct conf_opt *o;
     struct conf_sec *s;

     if (sec) {
          while (!SLIST_EMPTY(&sec->optlist)) {
               o = SLIST_FIRST(&sec->optlist);
               SLIST_REMOVE_HEAD(&sec->optlist, entry);
               free_opt(o);
          }
          while (!TAILQ_EMPTY(&sec->sub)) {
               s = TAILQ_FIRST(&sec->sub);
               TAILQ_REMOVE(&sec->sub, s, entry);
               free_sec(s);
          }
          free(sec);
     }
     return NULL;
}

static struct conf_sec *
get_section(struct keyword **head)
{
     struct conf_sec *s;
     struct conf_opt *o;
     struct conf_sec *sub;
     struct keyword *kw = *head;

     s = zcalloc(sizeof(*s));
     s->name = kw->name;
     TAILQ_INIT(&s->sub);
     SLIST_INIT(&s->optlist);

     kw = kw->next;

     while (kw && kw->type != SEC_END) {
          switch (kw->type) {
               case INCLUDE:
                    if (!(kw = include(kw)))
                         return free_sec(s);
                    break;
               case SEC_START:
                    if (!(sub = get_section(&kw)))
                         return free_sec(s);
                    TAILQ_INSERT_TAIL(&s->sub, sub, entry);
                    s->nsub++;
                    break;
               case WORD:
                    if (!(o = get_option(&kw)))
                         return free_sec(s);
                    SLIST_INSERT_HEAD(&s->optlist, o, entry);
                    s->nopt++;
                    break;
               default:
                    syntax(kw, "syntax error");
                    return free_sec(s);
                    break;
          }
     }

     if (!kw || strcmp(kw->name, s->name)) {
          syntax(kw, "missing end section %s", s->name);
          return free_sec(s);
     }

     kw = kw->next;
     *head = kw;

     return s;
}

int
free_conf(void)
{
     struct conf_sec *s;
     struct keyword *kw, *nkw;
     struct files **f = NULL;
     int i, nf = 0;

     while (!TAILQ_EMPTY(&config)) {
          s = TAILQ_FIRST(&config);
          TAILQ_REMOVE(&config, s, entry);
          free_sec(s);
     }

     kw = keywords;

     while (kw) {
          nkw = kw->next;

          free(kw->name);

          for (i = 0; i < nf; i++) {
               if (f[i] == kw->file) {
                    if (!(f = realloc(f, sizeof(*f) * (++i))))
                         err(EXIT_FAILURE, "realloc");
                    f[i-1] = kw->file;
               }
          }

          kw = nkw;
     }

     if (nf > 0) {
          for (i = 0; i < nf; i++) {
               free(f[i]->name);
               free(f[i]);
          }
          free(f);
     }
     return -1;
}

int
get_conf(const char *filename)
{
     struct conf_sec *s;
     struct keyword *head, *kw;

     kw = head = parse_keywords(filename);

     if (!head)
          return -1; /* TODO ERREUR */

     keywords = head;

     TAILQ_INIT(&config);

     while (kw) {
          switch (kw->type) {
               case INCLUDE:
                    if (!(kw = include(kw)))
                         return free_conf();
                    break;
               case SEC_START:
                    if (!(s = get_section(&kw)))
                         return free_conf();
                    TAILQ_INSERT_TAIL(&config, s, entry);
                    break;
               default:
                    syntax(kw, "out of any section");
                    return free_conf();
                    break;
          }
     }
     return 0;
}

