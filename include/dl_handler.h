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

extern const char* foo();
extern const char* bar();

void* my_dlopen(char* name);

void* my_dlsym(void* handler, char* func);

void* loader_plt_resolver(void *handler, int import_id);

void my_dlset_plt_resolve(void* handler, exported_table_t imported_symbols[]);