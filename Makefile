# -----------------------------------------------------------------------------
# IMAGEWTY-Tool Cross Platform Makefile
# -----------------------------------------------------------------------------

CC ?= gcc

BASE_CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude
CFLAGS ?= $(BASE_CFLAGS)
LDFLAGS ?=

BIN ?= imagewty-tool

SRC = \
    src/main.c \
    src/img_header.c \
    src/img_extract.c \
    src/img_repack.c \
    src/checksum.c \
    src/config_file.c \
    src/print_info.c

OBJ = $(SRC:.c=.o)

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# Platform targets
# -----------------------------------------------------------------------------

linux:
	$(MAKE) clean
	$(MAKE) BIN=imagewty-tool-linux

linux32:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(BASE_CFLAGS) -m32" LDFLAGS="-m32" BIN=imagewty-tool-linux-x86

win32:
	$(MAKE) clean
	$(MAKE) CC=i686-w64-mingw32-gcc BIN=imagewty-tool-win32.exe LDFLAGS="-static"

win64:
	$(MAKE) clean
	$(MAKE) CC=x86_64-w64-mingw32-gcc BIN=imagewty-tool-win64.exe LDFLAGS="-static"

macos-intel:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(BASE_CFLAGS) -arch x86_64" BIN=imagewty-tool-macos-x86_64

macos-arm:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(BASE_CFLAGS) -arch arm64" BIN=imagewty-tool-macos-arm64

android-armv7:
	$(MAKE) clean
	$(MAKE) CC=armv7a-linux-androideabi21-clang BIN=imagewty-tool-android-armv7

android-arm64:
	$(MAKE) clean
	$(MAKE) CC=aarch64-linux-android21-clang BIN=imagewty-tool-android-arm64

# -----------------------------------------------------------------------------
# Utility targets
# -----------------------------------------------------------------------------

clean:
	rm -f $(OBJ) imagewty-tool*

install: $(BIN)
	cp $(BIN) /usr/local/bin/$(BIN)

format:
	clang-format -i -style=file $(SRC) include/*.h

.PHONY: all clean install format \
        linux linux32 win32 win64 \
        macos-intel macos-arm \
        android-armv7 android-arm64
