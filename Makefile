include config.mk
include common.mk

OBJ = ${patsubst %.c,${O}/%.o,${SRCS}}

ifneq ($(findstring xrandr, ${LIBS}),)
CFLAGS+= -DHAVE_XRANDR
endif

ifneq ($(findstring xinerama, ${LIBS}),)
CFLAGS+= -DHAVE_XINERAMA
endif

ifneq ($(findstring imlib2, ${LIBS}),)
CFLAGS+= -DHAVE_IMLIB2
endif

ifndef CFLAGS_LIBS
CFLAGS_LIBS= $(shell pkg-config --cflags-only-I ${LIBS})
endif

ifndef LDFLAGS_LIBS
LDFLAGS_LIBS= $(shell pkg-config --libs ${LIBS})
endif

CFLAGS+= ${CFLAGS_LIBS}
LDFLAGS+= ${LDFLAGS_LIBS} -lpthread

all: options ${O}/wmfs ${O}/wmfs.1.gz

${O}/wmfs.1.gz: wmfs.1
	gzip -cn -9 $< > $@

${O}/%.o: %.c config.mk
	@if [ ! -d `dirname ${O}/$<` ]; then mkdir -p `dirname ${O}/$<`; fi
	@echo CC $<
	@${CC} -c ${CFLAGS} $< -o $@

options:
	@echo wmfs compile with ${LIBS}
	@echo - CFLAGS ${CFLAGS}
	@echo - LDFLAGS ${LDFLAGS}
	@echo - OUTPUT ${O}

	@if [ ! -d ${O} ]; then mkdir -p ${O}; fi

${O}/wmfs: ${OBJ} config.mk src/structs.h src/wmfs.h src/parse/parse.h
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@rm -f ${OBJ} ${O}/wmfs ${O}/wmfs.1.gz

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@install ${O}/wmfs ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@install -m 644 ${O}/wmfs.1.gz ${DESTDIR}${MANPREFIX}/man1/
	@echo installing xsession file to ${DESTDIR}${PREFIX}/share/xsessions
	@install -m 644 wmfs.desktop ${DESTDIR}${PREFIX}/share/xsessions

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/wmfs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/wmfs.1.gz
	@echo removing xsession file from ${DESTDIR}${PREFIX}/share/xsessions
	@rm -f ${DESTDIR}${PREFIX}/share/xsessions/wmfs.desktop



.PHONY: all clean install uninstall

