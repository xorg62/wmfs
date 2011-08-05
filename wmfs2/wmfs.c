/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *
 */

/* Standard */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <sys/queue.h>

/* Xlib */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

/* Macro */
#define FREE_LIST(type, head)                  \
     do {                                      \
          type *t;                             \
          while(!SLIST_EMPTY(&head)) {         \
               t = SLIST_FIRST(&head);         \
               SLIST_REMOVE_HEAD(&head, next); \
               free(t);                        \
          }                                    \
     } while(/* CONSTCOND */ 0);               \

/* Structure */
typedef unsigned int Flags;

typedef struct
{
     int x, y, w, h;
} Geo;

typedef struct Tag
{
     char *name;
     Screen *screen;
     Flags flags;
     Client *sel;
     SLIST_ENTRY(Tag) next;
} Tag;

typedef struct Screen
{
     Geo geo;
     Tag *seltag;
     SLIST_HEAD(, Tag) tags;
     SLIST_ENTRY(Screen) next;
} Screen;

typedef struct Client
{
     Tag *tag;
     Screen *screen;
     Geo *geo;
     Flags flags;
     SLIST_ENTRY(Client) next;
} Client;

typedef struct
{
     /* X11 stuffs */
     Display *dpy;
     Window root;
     int xscreen;
     GC gc;

     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, Screen) screen;
          SLIST_HEAD(, Client) client;
     } h;

     /*
      * Selected screen, from what you go everywhere; selected tag,
      * and then selected client.
      */
     Screen *screen;

} Wmfs;

/* Only global variable */
Wmfs *W;

/*
 * SCREEN
 */
void
screen_init(void)
{
     Geo g;

     g.x = 0;
     g.y = 0;
     g.w = DisplayWidth(W->dpy, W->xscreen);
     g.h = DisplayHeight(W->dpy, W->xscreen);

     SLIST_INIT(&W->h.screen);

     screen_new(g);
}

Screen*
screen_new(Geo g)
{
     Screen *s = calloc(sizeof(Screen));

     s->geo = g;
     s->seltag = NULL;

     SLIST_INIT(&s->tags);

     SLIST_INSERT_HEAD(s, &W->h.screen, next);

     W->screen = s;

     return s;
}

void
screen_free(void)
{
     FREE_LIST(Screen, W->h.screen);
}

/*
 * INIT
 */
void
init(void)
{
     /* X init */
     W->xscreen = DefaultScreen(W->dpy);
     W->gc      = DefaultGC(W->dpy, W->xscreen);
     W->root    = RootWindow(dpy, W->xscreen);

     /* Screen init */
     screen_init();

     return;
}

void
quit(void)
{
     /* X stuffs */
     XCloseDisplay(W->dpy);
     XFreeGC(W->gc);
     XFreeFontSet(dpy, W->font.fontset);

     screen_free();

     free(W);
}

int
main(int argc, char **argv)
{

     W = calloc(sizeof(Wmfs));


     /* Get X display */
     if(!(W->dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "%s: Can't open X server\n", argv[0]);
          exit(EXIT_FAILURE);
     }

     init();

     quit();

     return 1;
}

