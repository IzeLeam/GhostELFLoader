#include <stdlib.h>

#include "args_parser.h"

static char doc[] = "isos_loader -- Dynamic ELF loader";

static struct argp_option options[] = {
    { "file",         'f', "FILE", 0, "The ELF file to analyze" },
    { "to-inject",    'i', "FILE", 0, "The binary file containing the machine code to be injected" },
    { "section-name", 's', "NAME", 0, "The name of the section to create" },
    { "base-addr",    'b', "ADDR", 0, "The base address of the injected code" },
    { "modify-entry", 'm', "BOOL", 0, "Whether the entry point should be modified or not" },
    { 0 }
};

error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'h':
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
            break;
        case 'f':
            arguments->file = arg;
            break;
        case 'i':
            arguments->to_inject = arg;
            break;
        case 's':
            arguments->section_name = arg;
            break;
        case 'b':
            arguments->base_addr = arg;
            break;
        case 'm':
            arguments->modify_entry = atoi(arg);
            break;
        case ARGP_KEY_END:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = { options, parse_opt, 0, doc };