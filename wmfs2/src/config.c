/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "config.h"
#include "wmfs.h"
#include "parse.h"

#define CONFIG_DEFAULT_PATH ".config/wmfs/wmfsrc2" /* tmp */

static void
config_keybind(void)
{
     int i;
     size_t j, n;
     struct conf_sec *sec, **ks;
     struct opt_type *opt;
     Keybind *k;

     sec = fetch_section_first(NULL, "keys");
     ks = fetch_section(sec, "key");
     n = fetch_section_count(ks);

     SLIST_INIT(&W->h.keybind);

     for(i = 0; i < n; ++i)
     {
          opt = fetch_opt(ks[i], "", "mod");

          k = xcalloc(1, sizeof(Keybind));

          for(j = 0; j < fetch_opt_count(opt); ++j)
               k->mod |= modkey_keysym(opt[j].str);

          free(opt);

          k->keysym = XStringToKeysym(fetch_opt_first(ks[i], "None", "key").str);

          if(!(k->func = uicb_name_func(fetch_opt_first(ks[i], "", "func").str)))
          {
               warnx("configuration : Unknown Function \"%s\".", fetch_opt_first(ks[i], "", "func").str);
               k->func = uicb_spawn;
          }

          k->cmd = fetch_opt_first(ks[i], "", "cmd").str;

          SLIST_INSERT_HEAD(&W->h.keybind, k, next);

          k = NULL;
     }

     free(ks);
}

void
config_init(void)
{
     char *path;

     xasprintf(&path, "%s/"CONFIG_DEFAULT_PATH, getenv("HOME"));

     if(get_conf(path) == -1)
          errx(1, "parsing configuration file (%s) failed.", path);

     config_keybind();

     free(path);
}
