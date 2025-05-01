#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <key>\n", argv[0]);
        return 1;
    }

    encrypt_file(argv[1], argv[2], argv[3]);
    printf("File encrypted %s\n", argv[2]);
    return 0;
}