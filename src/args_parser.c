#include <stdlib.h>
#include <argp.h>

#include "args_parser.h"

static char doc[] = "isos_loader -- Dynamic ELF loader";
static char args_doc[] = "FILE FUNCTION...";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Verbose output", 0},
    {"key", 'k', "KEY", 0, "Encryption key", 0},
    { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments_t *arguments = state->input;

    switch (key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case 'k':
            arguments->key = arg;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                arguments->file = arg;
            } else {
                arguments->nb_functions++;
                arguments->functions[state->arg_num - 1] = arg;
            }
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 2)
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct arguments_t* parse_arguments(int argc, char **argv) {
    struct arguments_t *arguments = malloc(sizeof(struct arguments_t));
    arguments->functions = malloc(sizeof(char*) * (argc - 2));
    arguments->verbose = 0;
    arguments->file = NULL;
    arguments->key = NULL;
    arguments->nb_functions = 0;

    struct argp argp = { options, parse_opt, args_doc, doc };
    argp_parse(&argp, argc, argv, 0, 0, arguments);

    return arguments;
}

void free_arguments(struct arguments_t *arguments) {
    free(arguments->functions);
    free(arguments);
}