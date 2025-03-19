#include <stdlib.h>

#include "args_parser.h"

static char doc[] = "isos_loader -- Dynamic ELF loader";
static char args_doc[] = "FILE FUNCTION...";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    { 0 }
};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                arguments->file = arg;
            }
            else {
                arguments->nb_functions++;
                arguments->functions = realloc(arguments->functions, sizeof(char*) * (state->arg_num - 1));
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

struct argp argp = { options, parse_opt, args_doc, doc };