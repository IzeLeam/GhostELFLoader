#pragma once

extern const char* foo();
extern const char* bar();

void* my_dlopen(char* name);

void* my_dlsym(void* handler, char* func);

void* loader_plt_resolver(void *handler, int import_id);

void my_dlset_plt_resolve(void* handler, symbol_entry_t imported_symbols[]);