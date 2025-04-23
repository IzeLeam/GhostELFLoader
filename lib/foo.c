#include <stddef.h>

/*
    Exported functions
*/

typedef struct {
    char *name;
    void *addr;
} my_symbol_t;

const char* foo_exported() {
    return "present";
}

const char* bar_exported() {
    return "present";
}

my_symbol_t symbols[] = {
    {"foo_exported", foo_exported},
    {"bar_exported", bar_exported},
    {NULL, NULL}
};

my_symbol_t* entry = symbols;

/*
    Imported functions
*/

// Extern functions defined in the main.c file
extern const char* foo_imported();
extern const char* bar_imported();

const char* lib_foo_imported() {
    return foo_imported();
}
const char* lib_bar_imported() {
    return bar_imported();
}

