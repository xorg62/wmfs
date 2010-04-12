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

#define TOKEN(t)          \
     do {                         \
          kw->type = (t);     \
          kw->line = file.line; \
          TAILQ_INSERT_TAIL(&keywords, kw, entry); \
          kw = malloc(sizeof(*kw)); \
     } while (0)

#define NEW_WORD()                                             \
     do {                                                       \
          if (j > 0) {                                        \
               e->name[j] = '\0';                              \
               TAILQ_INSERT_TAIL(&stack, e, entry);     \
               e = malloc(sizeof(*e));                         \
               j = 0;                                             \
               TOKEN(WORD);                                   \
          }                                                       \
     } while (0)

struct conf_keyword {
     enum conf_type type;
     size_t line;
     TAILQ_ENTRY(conf_keyword) entry;
};

struct conf_stack {
     char name[BUFSIZ];
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
     SLIST_ENTRY(conf_opt) entry;
     size_t nval;
};

struct conf_sec {
     char *name;
     SLIST_HEAD(, conf_opt) optlist;
     SLIST_HEAD(, conf_sec) sub;
     SLIST_ENTRY(conf_sec) entry;
     size_t nopt;
     size_t nsub;
};

struct opt_type {
     long int num;
     float fnum;
     Bool bool;
     char *str;
};

void get_keyword(const char *, size_t);
void get_conf(void);
struct conf_sec **fetch_section(struct conf_sec *, char *);
struct opt_type *fetch_opt(struct conf_sec *, char *, char *);

#endif /* PARSE_H */
