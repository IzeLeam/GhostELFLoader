#include <stddef.h>

#include "isos-support.h"
#include "lib.h"

/*
    Exported functions
*/

const char* foo_exported() {
    return "present";
}

const char* bar_exported() {
    return "present";
}

/*
    Imported functions
*/

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

// Exported symbols
exported_table_t exported_symbols[] = {
    {"foo_exported", foo_exported},
    {"bar_exported", bar_exported},
    {"lib_foo_imported", lib_foo_imported},
    {"lib_bar_imported", lib_bar_imported},
    {NULL, NULL}
};

// Imported symbols
const char* imported_symbols[] = {
    "foo_imported",
    "bar_imported",
    NULL
};

void* loader_handle = NULL;
void* isos_trampoline = NULL;

// Struct for entry point
loader_entry_t loader_entry = {
    .exported = exported_symbols,
    .imported = imported_symbols,
    .plt_table = NULL,  // This is used in the my_dlset_plt_resolve to attribute the PLT table
    .trampoline = &isos_trampoline,
    .handle = &loader_handle
};

// Entry point for the loader
loader_entry_t* entry = &loader_entry;

/*
    PLT entries
*/

PLT_BEGIN
PLT_ENTRY(FOO_IMPORTED_ID, "foo_imported")
PLT_ENTRY(BAR_IMPORTED_ID, "bar_imported")