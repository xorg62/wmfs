# Customize below to fit your system
# x11 xft and freetype2 are REQUIRED, others are optionals
OPTIONS= xrandr xinerama imlib2

# If theses variables are defined, make will not call pkg-config
#
# Linux example
# CFLAGS_LIBS= -I/usr/include/freetype2
# LDFLAGS_LIBS= -lX11 -lXft -lfreetype -lXrandr -lXinerama -lImlib2
#
# FreeBSD example
# CFLAGS_LIBS= -I/usr/local/include -I/usr/local/include/freetype2
# LDFLAGS_LIBS= -L/usr/local/lib -lXft -lXrender -lfontconfig -lX11 -lfreetype -lXrandr -lXinerama -lImlib2
#
# OpenBSD example
# CFLAGS_LIBS= -I/usr/X11R6/include -I/usr/X11R6/include/freetype2 -I/usr/local/include
# LDFLAGS_LIBS= -L/usr/X11R6/lib -L/usr/local/lib -lXft -lfontconfig -lexpat -lXrandr -lXrender -lXinerama -lxcb -lpthread-subs -lXau -lXdmcp -lImlib2 -lfreetype -lz -lX11 -lXext -lm

# paths
PREFIX= /usr/local
MANPREFIX= ${PREFIX}/man
XDG_CONFIG_DIR= /usr/local/etc/xdg/wmfs

CFLAGS= -Wall
