#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "foo.h"
#include "handler.h"
#include "args_parser.h"
#include "elf.h"

int main(int argc, char **argv) {
    struct arguments arguments = {NULL, 0, 0, NULL};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.verbose) {
        printf("Verbose enabled\n");
        printf("File: %s\n", arguments.file);
        printf("Function(s):\n");
        for (int i = 0; i < arguments.nb_functions; i++) {
            printf("  %s\n", arguments.functions[i]);
        }
    }

    void* handle = my_dlopen(arguments.file);
    if (!handle) {
        dprintf(STDERR_FILENO, "Failed to load the shared library\n");
        return 1;
    }

    for (int i = 0; i < arguments.nb_functions; i++) {
        char** func = &arguments.functions[i];
        char* (*function)() = my_dlsym(handle, *func);
        if (!function) {
            dprintf(STDERR_FILENO, "Failed to find the %s function\n", *func);
            return 1;
        }
        if (arguments.verbose) {
            printf("%s() returned %s\n", *func, function());
        }
    }

    Elf64_Ehdr header;
    parse_elf_header(arguments.file, &header);

    Elf64_Phdr** pheaders = malloc(sizeof(Elf64_Phdr*));
    parse_program_headers(arguments.file, &header, pheaders);

    return 0;
}
