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

#include "wmfs.h"
#include <sys/queue.h>

#define INCLUDE_CMD "@include"
#define PARSE_MAX_LIST 32

#if defined(Bool)
#define bool_t Bool
#else
typedef enum { False, True } bool_t;
#endif /* Bool */

struct conf_opt {
     char *name;
     char *val[PARSE_MAX_LIST];
     size_t nval;
     bool_t used;
     int line;
     char *filename;
     SLIST_ENTRY(conf_opt) entry;
};

struct conf_sec {
     char *name;
     SLIST_HEAD(, conf_opt) optlist;
     TAILQ_HEAD(, conf_sec) sub;
     size_t nopt;
     size_t nsub;
     TAILQ_ENTRY(conf_sec) entry;
};

struct opt_type {
     long int num;
     float fnum;
     bool_t bool;
     char *str;
};

/*
 * Create config from file
 * return -1 on failure
 */
int get_conf(const char *);

/*
 * Print unused option name from section s (and subsections).
 * If s == NULL print unused option name for all config struct.
 */
void print_unused(struct conf_sec *s);

/*
 * Free the config struct.
 * WARNING: This make all string
 * returned by fetch_(opt|section)(_first) unusable.
 */
int free_conf(void);

/*
 * Get all subsection matching the given name on the given
 * section.
 * If section == NULL, return subsections from root section.
 * Return a NULL terminated array.
 * Subsections are returned in order as they are in config file
 * WARNING : This MUST be free() after use.
 */
struct conf_sec **fetch_section(struct conf_sec *, char *);

/*
 * Get first subsection  matching the given name
 * on the given section. (first found on the file)
 */
struct conf_sec *fetch_section_first(struct conf_sec *, char *);

/*
 * Count member of a conf_sec **
 */
size_t fetch_section_count(struct conf_sec **);

/*
 * Return all options matching the given name on the given subsection.
 * If none match or section == NULL return opt_type build with the
 * given default param.
 * WARNING: This MUST be free() after use.
 * WARNING: The string member is directly taken from the config struct.
 * WARNING: Returned in reverse order as they are in config file.
 * (I think the last option MUST overwrite all others)
 */
struct opt_type fetch_opt_first(struct conf_sec *, char *, char *);

/*
 * Get first (last in config file) option matching the given name
 * on the given section.
 * WARNING: The string member is directly taken from the config struct.
 */
struct opt_type *fetch_opt(struct conf_sec *, char *, char *);

/*
 * Count member of a opt_type *
 */
size_t fetch_opt_count(struct opt_type *);

#endif /* PARSE_H */
/* vim: et ts=5 sts=5 sw=5:
 */