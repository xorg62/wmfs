#
# Makefile to use with BSDBuild
#
# Copyright (c) 2011, David "markand" Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

TOP= .
PROG= wmfs
SRCS= src/barwin.c    \
      src/cfactor.c   \
      src/client.c    \
      src/config.c    \
      src/draw.c      \
      src/event.c     \
      src/ewmh.c      \
      src/frame.c     \
      src/getinfo.c   \
      src/infobar.c   \
      src/init.c      \
      src/launcher.c  \
      src/layout.c    \
      src/menu.c      \
      src/mouse.c     \
      src/parse_api.c \
      src/parse.c     \
      src/screen.c    \
      src/split.c     \
      src/status.c    \
      src/systray.c   \
      src/tag.c       \
      src/util.c      \
      src/viwmfs.c    \
      src/color.c     \
      src/wmfs.c

include ${TOP}/Makefile.config
include ${TOP}/mk/build.prog.mk

CFLAGS+=  -Wall

# Include dirs
CFLAGS+= ${X11_CFLAGS}
CFLAGS+= ${XFT_CFLAGS}
CFLAGS+= ${FREETYPE_CFLAGS}
CFLAGS+= ${PTHREADS_CFLAGS}
CFLAGS+= ${XINERAMA_CFLAGS}
CFLAGS+= ${XRANDR_CFLAGS}
CFLAGS+= ${IMLIB2_CFLAGS}

# XDG Dir and WMFS version
CFLAGS+= -DXDG_CONFIG_DIR=\"${XDG_CONFIG_DIR}\"
CFLAGS+= -DWMFS_VERSION=\"201104\"

# Libs dirs
LIBS+=  ${X11_LIBS}
LIBS+=  ${XFT_LIBS}
LIBS+=  ${FREETYPE_LIBS}
LIBS+=  ${PTHREADS_LIBS}
LIBS+=  ${XINERAMA_LIBS}
LIBS+=  ${XRANDR_LIBS}
LIBS+=  ${IMLIB2_LIBS}

# Install man and wmfsrc
install:
	${INSTALL_DATA} wmfs.1 ${MANDIR}/man1
	${INSTALL_DATA} wmfsrc ${XDG_CONFIG_DIR}/wmfs/
