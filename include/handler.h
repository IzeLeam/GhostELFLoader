#pragma once

void* my_dlopen(char* name);

void* my_dlsym(void* handle, char* func);