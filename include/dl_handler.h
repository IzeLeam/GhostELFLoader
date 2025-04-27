#pragma once

typedef struct {
    char *name;
    void *addr;
} exported_table_t;

typedef struct {
    exported_table_t* exported;
    const char** imported;
} loader_entry_t;

void* my_dlopen(char* name);

void* my_dlsym(void* handle, char* func);