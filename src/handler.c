#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void* my_dlopen(char* name) {
    return dlopen(name, RTLD_NOW);
}

int* my_dlsym(void* handle, char* func) {
    return dlsym(handle, func);
}