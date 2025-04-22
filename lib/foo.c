#include <stddef.h>

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
const char* foo_imported() {
    import();
    return "present";
}

const char* bar_imported() {
    import();
    return "present";
}
*/
