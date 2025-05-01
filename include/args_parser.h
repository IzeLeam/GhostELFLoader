#pragma once

struct arguments_t {
    char *file;
    int verbose;
    int nb_functions;
    char **functions;
    char *key;
};

struct arguments_t* parse_arguments(int argc, char **argv);
void free_arguments(struct arguments_t *arguments);