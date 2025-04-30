#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdarg.h>

#include "lib.h"
#include "dl_handler.h"
#include "args_parser.h"

static struct arguments_t arguments = {NULL, 0, 0, NULL};

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

/**
 * Debug function to print messages if verbose mode is enabled
 * 
 * @param format The format string
 * @param ... The arguments to format
 */
void debug(const char *format, ...) {
    if (arguments.verbose) {
        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}

int main(int argc, char **argv) {

    // Parsing command line arguments
    arguments.functions = malloc(sizeof(char*) * (argc - 2));
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.file == NULL) {
        perror("File not specified");
        return 1;
    }

    debug("\nVerbose enabled\n");
    debug("File: %s\n", arguments.file);
    debug("Function(s):\n");
    for (int i = 0; i < arguments.nb_functions; i++) {
        debug("  %s\n", arguments.functions[i]);
    }
    
    // Load the shared library
    void* handle = my_dlopen(arguments.file);
    if (!handle) {
        dprintf(STDERR_FILENO, "Failed to load the shared library\n");
        return 1;
    }

    // Resolve the symbols for the shared library
    my_dlset_plt_resolve(handle, imported_symbols);

    for (int i = 0; i < arguments.nb_functions; i++) {
        char** func = &arguments.functions[i];
        char* (*function)() = my_dlsym(handle, *func);
        if (!function) {
            dprintf(STDERR_FILENO, "Failed to find the %s function\n", *func);
            return 1;
        }
        printf("%s() returned %s\n", *func, function());
    }
    return 0;
}
