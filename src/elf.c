#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "elf.h"
#include "args_parser.h"

/**
 * Print the ELF header properties
 * 
 * @param header The ELF header structure
 */
static void print_elf_header(Elf64_Ehdr *header) {
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

/**
 * Verify ELF header properties
 * 
 * @param header The ELF header structure
 * 
 * @note This function will exit the program if the header is invalid
 */
static void check_elf_header(Elf64_Ehdr* header) {
    // Assert ELF Magic number
    if ((header->e_ident[EI_MAG0] != ELFMAG0)
        || (header->e_ident[EI_MAG1] != ELFMAG1)
        || (header->e_ident[EI_MAG2] != ELFMAG2)
        || (header->e_ident[EI_MAG3] != ELFMAG3)) {
        dprintf(STDERR_FILENO, "Invalid ELF magic number\n");
        exit(1);
    }

    // Assert ELF class is 64-bit
    if (header->e_ident[4] != ELFCLASS64) {
        dprintf(STDERR_FILENO, "Invalid ELF class, expected 64-bit\n");
        exit(1);
    }

    // Assert ELF type is shared object
    if (header->e_type != ET_DYN) {
        dprintf(STDERR_FILENO, "Invalid ELF type, expected shared object\n");
        exit(1);
    }

    // Assert ELF header size equals the size of the structure
    if (header->e_ehsize != sizeof(Elf64_Ehdr)) {
        dprintf(STDERR_FILENO, "Invalid ELF header size\n");
        exit(1);
    }

    // Assert program header size equals the size of the structure
    if (header->e_phentsize != sizeof(Elf64_Phdr)) {
        dprintf(STDERR_FILENO, "Invalid program header size\n");
        exit(1);
    }

    // Assert positive number of program headers
    if (header->e_phnum == 0) {
        dprintf(STDERR_FILENO, "Invalid number of program headers\n");
        exit(1);
    }
}

/**
 * Parse the ELF header of a file
 * 
 * @param fd The file descriptor of the file
 * @param header The ELF header structure as buffer
 */
void parse_elf_header(int fd, Elf64_Ehdr* header) {
    ssize_t size = read(fd, header, sizeof(Elf64_Ehdr));
    if (size < 0) {
        dprintf(STDERR_FILENO, "Failed to read the file\n");
        exit(1);
    }

    if (arguments.verbose) {
        print_elf_header(header);
    }

    check_elf_header(header);
}

/**
 * Convert program header flags to a string (R, W, X)
 */
static void get_phdr_flags(Elf64_Word flags, char* str) {
    str[0] = (flags & PF_R) ? 'R' : ' ';
    str[1] = (flags & PF_W) ? 'W' : ' ';
    str[2] = (flags & PF_X) ? 'E' : ' ';
    str[3] = '\0';
}

/**
 * Print a program header in readelf-like format
 * 
 * @param header The program header structure
 */
static void print_program_header(Elf64_Phdr* header) {
    char flags_str[4];
    get_phdr_flags(header->p_flags, flags_str);
    printf("  %-14s 0x%016lx 0x%016lx 0x%016lx\n",
           header->p_type == PT_LOAD ? "LOAD" : "UNKOWN", header->p_offset, header->p_vaddr, header->p_paddr);
    printf("                 0x%016lx 0x%016lx  %-4s   0x%lx\n",
           header->p_filesz, header->p_memsz, flags_str, header->p_align);
}

/**
 * Print the program headers
 * 
 * @param pheaders The program headers to print
 * @param nb_seg The number of loadable program headers
 */
void print_program_headers(Elf64_Phdr* pheaders, int nb_seg) {
    printf("Program header:\n");
    printf("  Type           Offset             VirtAddr          PhysAddr\n");
    printf("                 FileSiz           MemSiz            Flags  Align\n");
    for (int i = 0; i < nb_seg; i++) {
        print_program_header(&pheaders[i]);
    }
}

/**
 * Verify program headers properties
 * 
 * @param eheader The ELF header structure
 * @param pheaders The program headers to check
 * @param nb_seg The number of loadable program headers
 * 
 * @note This function will exit the program if one property is invalid
 */
static void check_program_headers(Elf64_Ehdr* eheader, Elf64_Phdr* pheaders, int nb_seg) {
    if (nb_seg == 0) {
        dprintf(STDERR_FILENO, "No program header found\n");
        exit(1);
    }

    // Assert that the first load segment is the first program header
    if (pheaders[0].p_offset >= eheader->e_phoff || pheaders[0].p_filesz < (eheader->e_phnum * sizeof(Elf64_Phdr))) {
        dprintf(STDERR_FILENO, "First load segment does not cover all program headers\n");
        exit(1);
    }

    // Assert that load segments are in ascending order and do not overlap each other
    for (int i = 1; i < nb_seg; i++) {
        if (pheaders[i].p_vaddr <= pheaders[i - 1].p_vaddr) {
            dprintf(STDERR_FILENO, "Load segments are not in ascending order\n");
            exit(1);
        }
        if (pheaders[i].p_vaddr < pheaders[i - 1].p_vaddr + pheaders[i - 1].p_memsz) {
            dprintf(STDERR_FILENO, "Load segments overlap\n");
            exit(1);
        }
    }
}

/**
 * Compute the total size of the segments
 * 
 * @param pheaders The program headers
 * @param nb_seg The number of loadable program headers
 * 
 * @return The total size of the segments
 */
int compute_total_size(Elf64_Phdr* pheaders, int nb_seg) {
    return (pheaders)[nb_seg - 1].p_vaddr + (pheaders)[nb_seg - 1].p_memsz - (pheaders)[0].p_vaddr;
}

/**
 * Parse the program headers of a file
 * 
 * @param fd The file descriptor of the file
 * @param eheader The ELF header structure
 * @param pheaders The program headers as buffer
 * 
 * @return The number of loadable program headers
 * 
 * @note This function will allocate memory for the program headers
 */
int parse_program_headers(int fd, Elf64_Ehdr* eheader, Elf64_Phdr** pheaders) {
    int nb_seg = 0;
    *pheaders = NULL;

    for (int i = 0; i < eheader->e_phnum; i++) {
        lseek(fd, eheader->e_phoff + i * sizeof(Elf64_Phdr), SEEK_SET);

        Elf64_Phdr pheader;
        ssize_t size = read(fd, &pheader, sizeof(Elf64_Phdr));
        if (size < 0) {
            dprintf(STDERR_FILENO, "Program parser: Failed to read the file\n");
            exit(1);
        }

        if (pheader.p_type != PT_LOAD) {
            continue;
        }

        Elf64_Phdr *guard = realloc(*pheaders, sizeof(Elf64_Phdr) * (nb_seg + 1));
        if (!guard) {
            dprintf(STDERR_FILENO, "Program parser: Failed to allocate memory\n");
            exit(1);
        }
        *pheaders = guard;
        (*pheaders)[nb_seg] = pheader;
        nb_seg++;
    }

    if (arguments.verbose) {
        print_program_headers(*pheaders, nb_seg);
    }

    check_program_headers(eheader, *pheaders, nb_seg);

    return nb_seg;
}
