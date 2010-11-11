.include "config.mk"
.include "common.mk"

PROG= wmfs
MAN1= wmfs.1

.for lib in xrandr xinerama imlib2
.if !empty(LIBS:M${lib})
CFLAGS+= -DHAVE_${lib:U}
.endif
.endfor

.if !defined(CFLAGS_LIBS)
CFLAGS_LIBS!= pkg-config --cflags-only-I ${LIBS}
.endif

.if !defined(LDFLAGS_LIBS)
LDFLAGS_LIBS!= pkg-config --libs ${LIBS}
.endif

CFLAGS+= ${CFLAGS_LIBS}
LDADD+= ${LDFLAGS_LIBS} -lpthread

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@install ${.OBJDIR}/wmfs ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@install -m 644 ${.OBJDIR}/wmfs.1.gz ${DESTDIR}${MANPREFIX}/man1/
	@echo installing xsession file to ${DESTDIR}${PREFIX}/share/xsessions
	@mkdir -p ${DESTDIR}${PREFIX}/share/xsessions
	@install -m 644 ${.CURDIR}/wmfs.desktop ${DESTDIR}${PREFIX}/share/xsessions/
	@echo installing default config file to ${DESTDIR}${XDG_CONFIG_DIR}
	@mkdir -p ${DESTDIR}${XDG_CONFIG_DIR}
	@install -m 444 ${.CURDIR}/wmfsrc ${DESTDIR}${XDG_CONFIG_DIR}

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/wmfs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/wmfs.1.gz
	@echo removing xsession file from ${DESTDIR}${PREFIX}/share/xsessions/
	@rm -f ${DESTDIR}${PREFIX}/share/xsessions/wmfs.desktop
	@echo removing config file from ${DESTDIR}${XDG_CONFIG_DIR}
	@rm -f ${DESTDIR}${XDG_CONFIG_DIR}/wmfsrc


.include <bsd.prog.mk>

.c.o: config.mk
	@if [ ! -d `dirname ${.TARGET}` ]; then mkdir -p `dirname ${.TARGET}`; fi
	@echo CC ${.IMPSRC}
	@${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
