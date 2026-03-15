# -----------------------------------------------------------------------------
# Makefile for IMAGEWTY-Tool
# -----------------------------------------------------------------------------
# This Makefile compiles the IMAGEWTY-Tool project (a tool to analyze, unpack
# and repack Allwinner firmware images).
#
# Targets:
#   all       - Build the executable
#   clean     - Remove all build artifacts
#   install   - Install the binary to /usr/local/bin
#   uninstall - Remove the binary from /usr/local/bin
#   cleanobj  - Remove only object files
#   format    - Format all source/header files using clang-format
#   debug     - Build with sanitizers and debug info
# -----------------------------------------------------------------------------

# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude
LDFLAGS = 

# Source files and objects
SRC = \
    src/main.c \
    src/img_header.c \
    src/img_extract.c \
    src/img_repack.c \
    src/checksum.c \
    src/config_file.c \
    src/print_info.c

OBJ = $(SRC:.c=.o)

# Binary name
BIN = imagewty-tool

# Default target: build the binary
all: $(BIN)

# Link the binary from object files
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

# Remove all object files and the binary
clean:
	rm -f $(OBJ) $(BIN)

# Install the binary to /usr/local/bin (requires sudo)
install: $(BIN)
	install -m 755 $(BIN) /usr/local/bin/$(BIN)

# Remove the binary from /usr/local/bin (requires sudo)
uninstall:
	rm -f /usr/local/bin/$(BIN)

# Remove only object files
cleanobj:
	rm -f $(OBJ)

# Automatically format all source and header files using clang-format
format:
	clang-format -i -style=file $(SRC) include/*.h

# Build with sanitizers and debug info (for development)
debug: CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -O0 -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer
debug: LDFLAGS = -fsanitize=address,undefined
debug: clean $(BIN)

.PHONY: all clean install uninstall cleanobj format debug
