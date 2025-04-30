#pragma once

#include <argp.h>

struct arguments_t {
    char *file;
    int verbose;
    int nb_functions;
    char **functions;
    char* key;
};

extern error_t parse_opt(int key, char *arg, struct argp_state *state);

extern struct argp argp;
