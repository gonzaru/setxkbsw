CC     ?= cc
CFLAGS ?=
STRIP  ?= strip

BIN_DIR := bin
TARGET  := $(BIN_DIR)/setxkbsw
SRC     := setxkbsw.c

VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || echo dev)

CFLAGS   += -Wno-implicit-function-declaration -Wno-int-conversion
CPPFLAGS += -DSETXKBSW_VERSION=\"$(VERSION)\"

PKG_OK := $(shell command -v pkg-config >/dev/null 2>&1 && pkg-config --exists x11 xkbfile && echo yes || echo no)

ifeq ($(PKG_OK),yes)
  CPPFLAGS += $(shell pkg-config --cflags x11 xkbfile)
  LIBS     := $(shell pkg-config --libs   x11 xkbfile)
else
  # *BSD fallback
  LIBS     := -L/usr/X11R6/lib -lX11 -lxkbfile
endif

.PHONY: all clean rebuild

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $@

$(TARGET): $(SRC) Makefile | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(SRC) $(LIBS)
	-$(STRIP) $@

clean:
	rm -rf $(BIN_DIR)

rebuild:
	$(MAKE) clean
	$(MAKE) all
