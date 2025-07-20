#!/bin/sh

set -eu

CC=${CC:-cc}
: "${CFLAGS:=}"
CFLAGS="$CFLAGS -Wno-implicit-function-declaration -Wno-int-conversion"
SETXKBSW_VERSION=$(git rev-parse --short HEAD 2>/dev/null || echo dev)
CFLAGS="$CFLAGS -DSETXKBSW_VERSION=\"${SETXKBSW_VERSION}\""

if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists x11 xkbfile
then
    CFLAGS="$CFLAGS $(pkg-config --cflags x11 xkbfile)"
    LIBS="$(pkg-config --libs   x11 xkbfile)"
else
    # *BSD
    LIBS='-L/usr/X11R6/lib -lX11 -lxkbfile'
fi

mkdir -p bin
"$CC" $CFLAGS -o bin/setxkbsw setxkbsw.c $LIBS
strip bin/setxkbsw
