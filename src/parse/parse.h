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

#ifndef PARSE_H
#define PARSE_H

#include <sys/queue.h>

#if !defined(WMFS_H)
typedef enum { False, True } Bool;
#endif

enum conf_type { SEC_START, SEC_END, WORD, EQUAL, LIST_START, LIST_END, NONE };

#define TOKEN(t)                                   \
     do {                                          \
          kw->type = (t);                          \
          TAILQ_INSERT_TAIL(&keywords, kw, entry); \
          kw = malloc(sizeof(*kw));                \
     } while (0)

#define NEW_WORD()                                  \
     do {                                           \
          if (j > 0) {                              \
               e->name[j] = '\0';                   \
               e->line = file.line;                 \
               TAILQ_INSERT_TAIL(&stack, e, entry); \
               e = malloc(sizeof(*e));              \
               j = 0;                               \
               TOKEN(WORD);                         \
          }                                         \
     } while (0)

struct conf_keyword {
     enum conf_type type;
     TAILQ_ENTRY(conf_keyword) entry;
};

struct conf_stack {
     char name[BUFSIZ];
     int line;
     TAILQ_ENTRY(conf_stack) entry;
};

struct conf_state {
     Bool quote;
     Bool comment;
     char quote_char;
};

struct conf_opt {
     char *name;
     char *val[10];
     size_t nval;
     Bool used;
     int line;
     SLIST_ENTRY(conf_opt) entry;
};

struct conf_sec {
     char *name;
     SLIST_HEAD(, conf_opt) optlist;
     SLIST_HEAD(, conf_sec) sub;
     size_t nopt;
     size_t nsub;
     SLIST_ENTRY(conf_sec) entry;
};

struct opt_type {
     long int num;
     float fnum;
     Bool bool;
     char *str;
};

int get_conf(const char *);
void print_unused(struct conf_sec *s);
void free_conf(struct conf_sec *s);

struct conf_sec **fetch_section(struct conf_sec *, char *);
struct conf_sec *fetch_section_first(struct conf_sec *, char *);
size_t fetch_section_count(struct conf_sec **);

struct opt_type fetch_opt_first(struct conf_sec *, char *, char *);
struct opt_type *fetch_opt(struct conf_sec *, char *, char *);
size_t fetch_opt_count(struct opt_type *);

#endif /* PARSE_H */
