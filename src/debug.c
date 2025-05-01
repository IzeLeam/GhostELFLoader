#include <stdio.h>
#include <stdarg.h>

static int debug_enabled = 0;

void set_debug(int enabled) {
    debug_enabled = enabled;
}

void debug(const char *format, ...) {
    if (debug_enabled) {
        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}