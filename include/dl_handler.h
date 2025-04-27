#pragma once

typedef struct {
    char *name;
    void *addr;
} exported_table_t;

typedef struct {
    exported_table_t* exported;
    const char** imported;
    exported_table_t* plt_table;
} loader_entry_t;

extern const char* foo_imported();
extern const char* bar_imported();

void* my_dlopen(char* name);

void* my_dlsym(void* handler, char* func);

void* loader_plt_resolver(void *handler, int import_id);

void my_dlset_plt_resolve(void* handler);