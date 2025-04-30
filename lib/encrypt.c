#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void xor_encrypt(const char *input_path, const char *output_path, const char *key) {
    FILE *input = fopen(input_path, "rb");
    FILE *output = fopen(output_path, "wb");
    if (!input || !output) {
        perror("Failed to open files");
        exit(1);
    }

    size_t key_len = strlen(key);
    size_t i = 0;
    int c;

    while ((c = fgetc(input)) != EOF) {
        fputc(c ^ key[i % key_len], output);
        i++;
    }

    fclose(input);
    fclose(output);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <key>\n", argv[0]);
        return 1;
    }

    xor_encrypt(argv[1], argv[2], argv[3]);
    printf("File encrypted successfully: %s\n", argv[2]);
    return 0;
}