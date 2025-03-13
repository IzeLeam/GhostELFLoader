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
SRC_FILES=./src/loader.c ./src/handler.c

# TODO
# Uncomment this and initialize it to the correct path(s) to your source files if your project sources are not located in `src`.
#vpath %.c path/to/src

all:
	@echo "succes"