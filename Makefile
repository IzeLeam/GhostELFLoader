include Makefile.config

all: $(SRC_FILES)
	gcc -o main src/loader.c -Iinclude