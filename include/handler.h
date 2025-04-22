#pragma once

typedef struct {
    char *name;
    void *addr;
} my_symbol_t;

void* my_dlopen(char* name);

void* my_dlsym(void* handle, char* func);