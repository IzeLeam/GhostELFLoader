#pragma once

#define FOO_IMPORTED_ID 0
#define BAR_IMPORTED_ID 1

typedef struct {
    char *name;
    void *addr;
} symbol_entry_t;

typedef struct {
    symbol_entry_t* exported;
    const char** imported;
    symbol_entry_t* plt_table;
    void** trampoline;
    void** handle;
    void** pltgot_entries;
} loader_entry_t;