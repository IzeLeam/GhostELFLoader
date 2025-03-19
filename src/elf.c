#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "elf.h"

void parse_elf_header(char* filename, Elf64_Ehdr* header) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open the file\n");
        exit(1);
    }

    char *buffer = malloc(sizeof(Elf64_Ehdr));
    ssize_t size = read(fd, buffer, sizeof(Elf64_Ehdr));
    if (size < 0) {
        fprintf(stderr, "Failed to read the file\n");
        exit(1);
    }

    memcpy(header, buffer, sizeof(Elf64_Ehdr));

    free(buffer);
    close(fd);
}

void check_elf_header(Elf64_Ehdr* header) {
    if ((header->e_ident[0] != 0x7f) || (header->e_ident[1] != 'E') || (header->e_ident[2] != 'L') || (header->e_ident[3] != 'F')) {
        fprintf(stderr, "Invalid ELF magic number\n");
        exit(1);
    }
    if (header->e_ident[4] != 2) {
        fprintf(stderr, "Invalid ELF class, expected 64-bit\n");
        exit(1);
    }
    if (header->e_type != 3) {
        fprintf(stderr, "Invalid ELF type, expected shared object\n");
        exit(1);
    }
    if (header->e_ehsize != sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "Invalid ELF header size\n");
        exit(1);
    }
    if (header->e_phnum == 0) {
        fprintf(stderr, "Invalid number of program headers\n");
        exit(1);
    }
}

void print_elf_header(Elf64_Ehdr *header) {
    printf("ELF header:\n");
    printf("  Magic: ");
    for (int i = 0; i < EI_NIDENT; i++) {
        printf("%02x ", header->e_ident[i]);
    }
    printf("\n");
    printf("  Type:                                 %d\n", header->e_type);
    printf("  Machine:                              %d\n", header->e_machine);
    printf("  Version:                              %d\n", header->e_version);
    printf("  Entry point address:                  %lu\n", header->e_entry);
    printf("  Start of program headers:             %lu (bytes into file)\n", header->e_phoff);
    printf("  Start of section headers:             %lu (bytes into file)\n", header->e_shoff);
    printf("  Flags:                                %d\n", header->e_flags);
    printf("  Size of this header:                  %d (bytes)\n", header->e_ehsize);
    printf("  Size of program headers:              %d (bytes)\n", header->e_phentsize);
    printf("  Number of program headers:            %d\n", header->e_phnum);
    printf("  Size of section headers:              %d (bytes)\n", header->e_shentsize);
    printf("  Number of section headers:            %d\n", header->e_shnum);
    printf("  Section header string table index:    %d\n", header->e_shstrndx);
}