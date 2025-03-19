# 
# This Makefile is for the ISOS project and ensure compatibility with the CI.
# Make sure to include this file in your root Makefile (i.e., at the top-level of your repository).
#

# TODO
# Initialize this variable to point to the directory holding your header if any.
# Otherwise, the CI will consider the top-level directory.
INCLUDE_DIR=./include

# TODO
# Initialize this variable with a space separated list of the paths to the loader source files (not the library).
# You can use some make native function such as wildcard if you want.
SRC_FILES=./src/loader.c ./src/handler.c ./src/args_parser.c ./src/elf.c
OBJ_FILES=$(SRC_FILES:.c=.o)

# TODO
# Uncomment this and initialize it to the correct path(s) to your source files if your project sources are not located in `src`.
#vpath %.c path/to/src

EMPTY=
SPACE=$(EMPTY) $(EMPTY)
COMMA_SPACE=,$(SPACE)

RED=\e[31m
GREEN=\e[1;32m
BLUE=\e[1;34m
PURPLE=\e[1;35m
END_COLOR=\e[0m

LIBS=
GCC_CFLAGS=-O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict
TARGET=isos_loader

$(TARGET): $(OBJ_FILES)
	@echo "$(PURPLE)Linking objects files [$(BLUE)$(subst $(SPACE),$(COMMA_SPACE),$(OBJ_FILES))$(PURPLE)] into binary $(GREEN)$@$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(LIBS) $(GCC_CFLAGS) -o $@ $^

%.o: %.c
	@echo "$(PURPLE)Building object file $@ for file $^$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(GCC_CFLAGS) -o $@ -c $^

lib: src/foo.c include/foo.h
	gcc -c src/foo.c -o foo.o
	gcc -shared -nostdlib -o lib/libfoo.so foo.o

clean:
	rm -rf src/*.o $(TARGET) lib/*.so

.PHONY: clean
