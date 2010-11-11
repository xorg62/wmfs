.include "config.mk"
.include "common.mk"

PROG= wmfs
MAN1= wmfs.1

.for lib in xrandr xinerama imlib2
HAVE_${lib:U}!= echo ${LIBS} | grep -q ${lib} && echo -DHAVE_${lib:U}
CFLAGS+= ${HAVE_${lib:U}}
.endfor

CFLAGS_LIB!= pkg-config --cflags-only-I ${LIBS}
LDFLAGS_LIB!= pkg-config --libs ${LIBS}

CFLAGS+= ${CFLAGS_LIB}
LDADD+= ${LDFLAGS_LIB} -lpthread

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@install ${.OBJDIR}/wmfs ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@install -m 644 ${.OBJDIR}/wmfs.1.gz ${DESTDIR}${MANPREFIX}/man1/
	@echo installing xsession file to ${DESTDIR}${PREFIX}/share/xsessions
	@install -m 644 ${.CURDIR}/wmfs.desktop ${DESTDIR}${PREFIX}/share/xsessions

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/wmfs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/wmfs.1.gz
	@echo removing xsession file from ${DESTDIR}${PREFIX}/share/xsessions
	@rm -f ${DESTDIR}${PREFIX}/share/xsessions/wmfs.desktop


.include <bsd.prog.mk>

.c.o: config.mk
	@if [ ! -d `dirname ${.TARGET}` ]; then mkdir -p `dirname ${.TARGET}`; fi
	@echo CC ${.IMPSRC}
	@${CC} ${CFLAGS} -c ${.IMPSRC} -o ${.TARGET}
