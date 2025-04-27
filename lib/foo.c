#include <stddef.h>

/*
    Exported functions
*/

typedef struct {
    char *name;
    void *addr;
} exported_table_t;

const char* foo_exported() {
    return "present";
}

const char* bar_exported() {
    return "present";
}

/*
    Imported functions
*/

#define FOO_IMPORTED_ID 0
#define BAR_IMPORTED_ID 1

extern const char* foo_imported();
extern const char* bar_imported();

const char* lib_foo_imported() {
    return foo_imported();
}

const char* lib_bar_imported() {
    return bar_imported();
}

/*
    Entry point for the loader
*/

typedef struct {
    exported_table_t* exported;
    const char** imported;
} loader_entry_t;


// Exported symbols
exported_table_t exported_symbols[] = {
    {"foo_exported", foo_exported},
    {"bar_exported", bar_exported},
    {NULL, NULL}
};

// Imported symbols
const char* imported_symbols[] = {
    "foo_imported",
    "bar_imported",
    NULL
};

// Struct for entry point
loader_entry_t loader_entry = {
    .exported = exported_symbols,
    .imported = imported_symbols
};

// Entry point for the loader
loader_entry_t* entry = &loader_entry;
