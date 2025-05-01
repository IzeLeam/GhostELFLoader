all: foo.c encrypt.c
	gcc -Wall -Wextra -fPIC -fvisibility=hidden -c foo.c -o foo.o -I../include
	gcc -nostdlib -shared -o libfoo.so foo.o -e entry

	gcc -o encrypt encrypt.c ../src/crypto.c -I../include

clean:
	rm -rf *.o *.so *.enc encrypt

.PHONY: clean
