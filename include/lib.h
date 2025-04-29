#pragma once

#define FOO_IMPORTED_ID 0
#define BAR_IMPORTED_ID 1

typedef struct {
    char *name;
    void *addr;
} exported_table_t;

typedef struct {
    exported_table_t* exported;
    const char** imported;
    exported_table_t* plt_table;
    void** trampoline;
    void** handle;
} loader_entry_t;