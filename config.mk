# wmfs version
VERSION= 201011

# Customize below to fit your system
# x11 xft and freetype2 are REQUIRED, others are optionals
LIBS= x11 xft freetype2 xrandr xinerama imlib2

# paths
PREFIX= /usr/local
MANPREFIX= ${PREFIX}/share/man
XDG_CONFIG_DIR= /usr/local/etc/xdg/wmfs

# flags
CFLAGS= -Wall -DXDG_CONFIG_DIR=\"${XDG_CONFIG_DIR}\"
CFLAGS+= $(shell pkg-config --cflags-only-I ${LIBS})
LDFLAGS= $(shell pkg-config --libs ${LIBS}) -lpthread
CFLAGS+= -DWMFS_VERSION=\"${VERSION}\"

CC = cc
