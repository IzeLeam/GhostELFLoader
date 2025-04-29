#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>

#include "dl_handler.h"
#include "args_parser.h"

/**
 * Imported function used in the shared library
 */
const char* foo() {
    return "present";
}

/**
 * Imported function used in the shared library
 */
const char* bar() {
    return "present";
}

exported_table_t imported_symbols[] = {
    {"foo_imported", foo},
    {"bar_imported", bar},
    {NULL, NULL}
};

struct arguments_t arguments = {NULL, 0, 0, NULL};

int main(int argc, char **argv) {

    arguments.functions = malloc(sizeof(char*) * (argc - 2));
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.file == NULL) {
        dprintf(STDERR_FILENO, "No file specified\n");
        return 1;
    }

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

    my_dlset_plt_resolve(handle, imported_symbols);

    #define FOO_IMPORTED_ID 0
    #define BAR_IMPORTED_ID 1

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

    return 0;
}
