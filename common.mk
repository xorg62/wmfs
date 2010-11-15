SRCS= \
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

# flags
CFLAGS+= ${C_FLAGS}
CFLAGS+= -DXDG_CONFIG_DIR=\"${XDG_CONFIG_DIR}\"
CFLAGS+= -DWMFS_VERSION=\"${VERSION}\"
LDFLAGS+= ${LD_FLAGS}

# build directory
O?= build
