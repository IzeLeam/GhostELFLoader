#pragma once

void* my_dlopen(char* name);

int* my_dlsym(void* handle, char* func);