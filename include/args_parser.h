#pragma once

#include <argp.h>

struct arguments {
    char *file;
    char *to_inject;
    char *section_name;
    char *base_addr;
    int modify_entry;
};

extern error_t parse_opt(int key, char *arg, struct argp_state *state);

extern struct argp argp;