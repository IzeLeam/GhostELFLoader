#include <stdio.h>
#include <stdlib.h>

#include "foo.h"
#include "handler.h"
#include "args_parser.h"

int import() {
    printf("Imported function called\n");
    return 0;
}

int main(int argc, char **argv) {
    struct arguments arguments = {NULL, NULL, NULL, NULL, 0};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

#ifdef DEBUG
    printf("File: %s\n", arguments.file);
    printf("To Inject: %s\n", arguments.to_inject);
    printf("Section Name: %s\n", arguments.section_name);
    printf("Base Address: %s\n", arguments.base_addr);
    printf("Modify Entry: %d\n", arguments.modify_entry);
#endif

    void* handle = my_dlopen(arguments.file);
    if (!handle) {
        fprintf(stderr, "Failed to load the shared library\n");
        return 1;
    }

    const char* (*foo_exported)() = my_dlsym(handle, "foo_exported");
    if (!foo_exported) {
        fprintf(stderr, "Failed to find the foo_exported function\n");
        return 1;
    }

    printf("foo_exported() returned %s\n", foo_exported());

    const char* (*foo_imported)() = my_dlsym(handle, "foo_imported");
    if (!foo_imported) {
        fprintf(stderr, "Failed to find the foo_imported function\n");
        return 1;
    }

    printf("foo_imported() returned %s\n", foo_imported());

    return 0;
}
