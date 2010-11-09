include config.mk

SRC= \
src/barwin.c \
src/client.c \
src/config.c \
src/draw.c \
src/event.c \
src/ewmh.c \
src/frame.c \
src/getinfo.c \
src/infobar.c \
src/init.c \
src/launcher.c \
src/layout.c \
src/menu.c \
src/mouse.c \
src/parse/api.c \
src/parse/parse.c \
src/screen.c \
src/status.c \
src/systray.c \
src/tag.c \
src/util.c \
src/viwmfs.c \
src/wmfs.c

OBJ = ${SRC:.c=.o}

ifneq ($(findstring xrandr, ${LIBS}),)
	CFLAGS+= -DHAVE_XRANDR
endif

ifneq ($(findstring xinerama, ${LIBS}),)
	CFLAGS+= -DHAVE_XINERAMA
endif

ifneq ($(findstring imlib2, ${LIBS}),)
	CFLAGS+= -DHAVE_IMLIB2
endif

all: options wmfs

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $< -o $@

options:
	@echo wmfs compile with ${LIBS}
	@echo - CFLAGS ${CFLAGS}
	@echo - LDFLAGS ${LDFLAGS}

wmfs: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@rm -f ${OBJ} wmfs

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@install wmfs ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@install -m 644 wmfs.1 ${DESTDIR}${MANPREFIX}/man1/
	@echo installing xsession file to ${DESTDIR}${PREFIX}/share/xsessions
	@install -m 644 wmfs.desktop ${DESTDIR}${PREFIX}/share/xsessions

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/wmfs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/wmfs.1
	@echo removing xsession file from ${DESTDIR}${PREFIX}/share/xsessions
	@rm -f ${DESTDIR}${PREFIX}/share/xsessions/wmfs.desktop


.PHONY: all clean install uninstall
