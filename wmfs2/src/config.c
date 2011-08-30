/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include "config.h"
#include "wmfs.h"
#include "parse.h"
#include "tag.h"
#include "screen.h"
#include "infobar.h"
#include "util.h"

#define CONFIG_DEFAULT_PATH ".config/wmfs/wmfsrc2" /* tmp */

static void
config_theme(void)
{
     size_t i, n;
     struct conf_sec *sec;

     /* [theme] */
     sec = fetch_section_first(NULL, "theme");

     /* bars */
     W->conf.theme.bars.fg = color_atoh(fetch_opt_first(sec, "#CCCCCC", "bars_fg").str);
     W->conf.theme.bars.bg = color_atoh(fetch_opt_first(sec, "#222222", "bars_bg").str);
     W->conf.theme.bars_width = fetch_opt_first(sec, "12", "bars_width").num;

     /*
      * Elements
      */
     W->conf.theme.tags_n.fg = color_atoh(fetch_opt_first(sec, "#CCCCCC", "tags_normal_fg").str);
     W->conf.theme.tags_n.bg = color_atoh(fetch_opt_first(sec, "#222222", "tags_normal_bg").str);
     W->conf.theme.tags_s.fg = color_atoh(fetch_opt_first(sec, "#222222", "tags_sel_fg").str);
     W->conf.theme.tags_s.bg = color_atoh(fetch_opt_first(sec, "#CCCCCC", "tags_sel_bg").str);
     W->conf.theme.tags_border_col = color_atoh(fetch_opt_first(sec, "#888888", "tags_border_color").str);
     W->conf.theme.tags_border_width = fetch_opt_first(sec, "0", "tags_border_width").num;
}

static void
config_bars(void)
{
     Scr33n *s;
     size_t i, n;
     struct conf_sec *sec, **ks;
     int screenid;
     char *elem;

     /* [bars] */
     sec = fetch_section_first(NULL, "bars");
     ks = fetch_section(sec, "bar");
     n = fetch_section_count(ks);

     /* [bar] */
     for(i = 0; i < n; ++i)
     {
          elem = fetch_opt_first(ks[i], "", "elements").str;
          screenid = fetch_opt_first(ks[i], "-1", "screen").num;

          SLIST_FOREACH(s, &W->h.screen, next)
               if(screenid == s->id || screenid == -1)
                    (Infobar*)infobar_new(s, elem);
     }

     free(ks);
}


static void
config_tag(void)
{
     Scr33n *s;
     Tag *t;
     size_t i, n;
     struct conf_sec *sec, **ks;
     char *name;
     int screenid;

     /* [tags] */
     sec = fetch_section_first(NULL, "tags");
     ks = fetch_section(sec, "tag");
     n = fetch_section_count(ks);

     /* [tag] */
     for(i = 0; i < n; ++i)
     {
          name = fetch_opt_first(ks[i], "tag", "name").str;
          screenid = fetch_opt_first(ks[i], "-1", "screen").num;

          SLIST_FOREACH(s, &W->h.screen, next)
               if(screenid == s->id || screenid == -1)
               {
                    t = tag_new(s, name);

                    /* Set first tag as seltag */
                    if(t == TAILQ_FIRST(&s->tags))
                         s->seltag = t;
               }
     }

     free(ks);
}

static void
config_keybind(void)
{
     int i;
     size_t j, n;
     struct conf_sec *sec, **ks;
     struct opt_type *opt;
     Keybind *k;

     /* [keys] */
     sec = fetch_section_first(NULL, "keys");
     ks = fetch_section(sec, "key");
     n = fetch_section_count(ks);

     SLIST_INIT(&W->h.keybind);

     /* [key] */
     for(i = 0; i < n; ++i)
     {
          opt = fetch_opt(ks[i], "", "mod");

          k = (Keybind*)xcalloc(1, sizeof(Keybind));

          for(j = 0; j < fetch_opt_count(opt); ++j)
               k->mod |= modkey_keysym(opt[j].str);

          free(opt);

          k->keysym = XStringToKeysym(fetch_opt_first(ks[i], "None", "key").str);

          if(!(k->func = uicb_name_func(fetch_opt_first(ks[i], "", "func").str)))
          {
               warnx("configuration: Unknown Function \"%s\".",
                         fetch_opt_first(ks[i], "", "func").str);
               k->func = uicb_spawn;
          }

          k->cmd = fetch_opt_first(ks[i], "", "cmd").str;

          SLIST_INSERT_HEAD(&W->h.keybind, k, next);
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

     config_theme();
     config_keybind();
     config_tag();
     config_bars();

     free(path);
}
