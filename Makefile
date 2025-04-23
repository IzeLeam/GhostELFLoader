# 
# This Makefile is for the ISOS project and ensure compatibility with the CI.
# Make sure to include this file in your root Makefile (i.e., at the top-level of your repository).
#

# Initialize this variable to point to the directory holding your header if any.
# Otherwise, the CI will consider the top-level directory.
INCLUDE_DIR=./include

# Initialize this variable with a space separated list of the paths to the loader source files (not the library).
# You can use some make native function such as wildcard if you want.
SRC_FILES=$(wildcard ./src/*.c)
OBJ_FILES=$(SRC_FILES:.c=.o)

EMPTY=
SPACE=$(EMPTY) $(EMPTY)
COMMA_SPACE=,$(SPACE)

RED=\e[31m
GREEN=\e[1;32m
BLUE=\e[1;34m
PURPLE=\e[1;35m
END_COLOR=\e[0m

GCC_CFLAGS=-O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict -g
TARGET=isos_loader

$(TARGET): $(OBJ_FILES)
	@echo "$(PURPLE)Linking objects files [$(BLUE)$(subst $(SPACE),$(COMMA_SPACE),$(OBJ_FILES))$(PURPLE)] into binary $(GREEN)$@$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(LIBS) $(GCC_CFLAGS) -o $@ $^

%.o: %.c
	@echo "$(PURPLE)Building object file $@ for file $^$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(GCC_CFLAGS) -o $@ -c $^

lib: lib/foo.c
	gcc -Wall -Wextra -fPIC -fvisibility=hidden -c lib/foo.c -o foo.o
	gcc -nostdlib -shared -o lib/libfoo.so foo.o -e entry
clean:
	rm -rf src/*.o $(TARGET) lib/*.so

.PHONY: clean
