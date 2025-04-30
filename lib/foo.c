#include <stddef.h>

#include "isos-support.h"
#include "lib.h"

/*
    Exported functions
*/

const char* foo_exported() {
    if (pltgot_entries[FOO_IMPORTED_ID] == NULL) {
        return "not present";
    }
    return "present";
}

const char* bar_exported() {
    return "present";
}

/*
    Imported functions
*/

extern const char* foo();
extern const char* bar();

const char* foo_imported() {
    return foo();
}

const char* bar_imported() {
    return bar();
}

/*
    Entry point for the loader
*/

// Exported symbols
exported_table_t exported_symbols[] = {
    {"foo_exported", foo_exported},
    {"bar_exported", bar_exported},
    {"foo_imported", foo_imported},
    {"bar_imported", bar_imported},
    {NULL, NULL}
};

// Imported symbols
const char* imported_symbols[] = {
    "foo",
    "bar",
    NULL
};

void* loader_handle = NULL;
void* isos_trampoline = NULL;

// Struct for entry point
loader_entry_t loader_entry = {
    .exported = exported_symbols,
    .imported = imported_symbols,
    .plt_table = NULL,  // This is only used in the my_dlset_plt_resolve to attribute the PLT table
    .trampoline = &isos_trampoline,
    .handle = &loader_handle,
    .pltgot_entries = pltgot_entries
};

// Entry point for the loader
loader_entry_t* entry = &loader_entry;

/*
    PLT entries
*/

BONUS_PLT_BEGIN
BONUS_PLT_ENTRY(FOO_IMPORTED_ID, foo)
BONUS_PLT_ENTRY(BAR_IMPORTED_ID, bar)