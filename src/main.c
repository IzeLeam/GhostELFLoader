#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "lib.h"
#include "dl_handler.h"
#include "args_parser.h"
#include "debug.h"

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

// Imported symbols associated with the function names
symbol_entry_t imported_symbols[] = {
    {"foo", foo},
    {"bar", bar},
    {NULL, NULL}
};

int main(int argc, char **argv) {

    // Parsing command line arguments
    struct arguments_t *arguments = parse_arguments(argc, argv);
    set_debug(arguments->verbose);

    if (arguments->file == NULL) {
        perror("File not specified");
        return 1;
    }

    debug("\nVerbose enabled\n");
    debug("File: %s\n", arguments->file);
    if (arguments->key) {
        debug("Decryption key: %s\n", arguments->key);
    }
    debug("Function(s):\n");
    for (int i = 0; i < arguments->nb_functions; i++) {
        debug("  %s\n", arguments->functions[i]);
    }
    
    // Load the shared library
    void* handle = my_dlopen(arguments->file, arguments->key);
    if (!handle) {
        dprintf(STDERR_FILENO, "Failed to load the shared library\n");
        return 1;
    }

    // Resolve the symbols for the shared library
    my_dlset_plt_resolve(handle, imported_symbols);

    for (int i = 0; i < arguments->nb_functions; i++) {
        char** func = &arguments->functions[i];
        char* (*function)() = my_dlsym(handle, *func);
        if (!function) {
            dprintf(STDERR_FILENO, "Failed to find the %s function\n", *func);
            return 1;
        }
        printf("%s() returned %s\n", *func, function());
    }
    return 0;
}
