#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "crypto.h"

/**
 * Encrypt a file using XOR encryption with the provided key
 * 
 * @param input_path The path to the input file
 * @param output_path The path to the output file
 * @param key The encryption key
 */
void encrypt_file(const char *input_path, const char *output_path, const char *key) {
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

/**
 * Decrypt the library using XOR encryption with the provided key
 * 
 * @param encrypted_path The path to the encrypted library
 * @param key The encryption key
 * 
 * @return The file descriptor of the decrypted library
 */
int decrypt_library(const char *encrypted_path, const char *key) {
    // Create a temporary file in the memory for the decrypted library
    int decrypted_fd = memfd_create("decrypted", 0);
    int encrypted_fd = open(encrypted_path, O_RDONLY);
    if (encrypted_fd < 0) {
        perror("Failed to open encrypted file");
        return -1;
    }

    size_t key_len = strlen(key);
    size_t i = 0;
    char c;

    // Read and decrypt the file byte by byte
    while (read(encrypted_fd, &c, 1) > 0) {
        c ^= key[i % key_len];
        if (write(decrypted_fd, &c, 1) != 1) {
            perror("Failed to write to decrypted file");
            close(encrypted_fd);
            close(decrypted_fd);
            return -1;
        }
        i++;
    }

    close(encrypted_fd);
    // Reset the file seek pointer
    lseek(decrypted_fd, 0, SEEK_SET);

    return decrypted_fd;
}