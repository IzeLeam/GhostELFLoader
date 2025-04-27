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
const char* foo_imported() {
    return "present";
}

/**
 * Imported function used in the shared library
 */
const char* bar_imported() {
    return "present";
}

// Handler used by the loader
void* loader_handle = NULL;

struct arguments_t arguments = {NULL, 0, 0, NULL};

int main(int argc, char **argv) {

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

    loader_handle = handle;

    my_dlset_plt_resolve(handle);
    if (arguments.verbose) {
        exported_table_t* plt = ((loader_entry_t*)handle)->plt_table;
        printf("PLT resolver :\n");
        for (int i = 0; plt[i].name != NULL; i++) {
            printf("\t- %s at %p\n", plt[i].name, plt[i].addr);
        }
    }

    #define FOO_IMPORTED_ID 0
    #define BAR_IMPORTED_ID 1

    void* foo = loader_plt_resolver(handle, FOO_IMPORTED_ID);
    if (!foo) {
        dprintf(STDERR_FILENO, "Failed to resolve foo_imported\n");
        return 1;
    }
    if (arguments.verbose) {
        printf("foo_imported() found at %p\n", foo);
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

    return 0;
}
