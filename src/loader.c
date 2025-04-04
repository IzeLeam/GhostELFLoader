#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>

#include "foo.h"
#include "handler.h"
#include "args_parser.h"
#include "elf.h"

void* load_segments(int fd, Elf64_Phdr* pheaders, int nb_seg, int total_size) {
    size_t page_size = sysconf(_SC_PAGESIZE);

    // Déterminer min_vaddr et max_vaddr
    Elf64_Addr min_vaddr = (Elf64_Addr)-1;
    Elf64_Addr max_vaddr = 0;
    for (int i = 0; i < nb_seg; i++) {
        if (pheaders[i].p_vaddr < min_vaddr)
            min_vaddr = pheaders[i].p_vaddr;
        if (pheaders[i].p_vaddr + pheaders[i].p_memsz > max_vaddr)
            max_vaddr = pheaders[i].p_vaddr + pheaders[i].p_memsz;
    }

    // Aligner min_vaddr sur une page et calculer la taille
    Elf64_Addr aligned_min_vaddr = min_vaddr & ~(page_size - 1);

    // Réserver l’espace mémoire avec mmap (PROT_NONE par défaut)
    void* base = mmap((void*)0x00, total_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Calcul du base_address
    Elf64_Addr base_address = (Elf64_Addr)base - aligned_min_vaddr;
    printf("Base address: 0x%lx\n", base_address);

    // Charger chaque segment
    for (int i = 0; i < nb_seg; i++) {
        Elf64_Phdr* ph = &pheaders[i];

        Elf64_Addr segment_vaddr = ph->p_vaddr + base_address;
        size_t segment_filesz = ph->p_filesz;
        size_t segment_memsz = ph->p_memsz;

        // Aligner segment_vaddr et p_offset sur une page
        Elf64_Addr aligned_vaddr = segment_vaddr & ~(page_size - 1);
        off_t aligned_offset = ph->p_offset & ~(page_size - 1);
        size_t extra_offset = segment_vaddr - aligned_vaddr;

        // Mapper le segment en mémoire
        void* mapped = mmap((void*)aligned_vaddr, segment_filesz + extra_offset,
                            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, aligned_offset);
        if (mapped == MAP_FAILED) {
            perror("mmap segment failed");
            exit(1);
        }

        // Remplir la section BSS avec des zéros (p_filesz < p_memsz)
        if (segment_memsz > segment_filesz) {
            void* bss_start = (void*)(segment_vaddr + segment_filesz);
            size_t bss_size = segment_memsz - segment_filesz;
            memset(bss_start, 0, bss_size);
        }

        // Appliquer les bonnes permissions
        int prot = 0;
        if (ph->p_flags & PF_R) prot |= PROT_READ;
        if (ph->p_flags & PF_W) prot |= PROT_WRITE;
        if (ph->p_flags & PF_X) prot |= PROT_EXEC;

        if (mprotect((void*)aligned_vaddr, segment_memsz + extra_offset, prot) < 0) {
            perror("mprotect failed");
            exit(1);
        }
    }

    return (void*)base_address;
}

struct arguments_t arguments = {NULL, 0, 0, NULL};

int main(int argc, char **argv) {

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.verbose) {
        printf("Verbose enabled\n");
        printf("File: %s\n", arguments.file);
        printf("Function(s):\n");
        for (int i = 0; i < arguments.nb_functions; i++) {
            printf("  %s\n", arguments.functions[i]);
        }
    }

    void* handle = my_dlopen(arguments.file);
    if (!handle) {
        dprintf(STDERR_FILENO, "Failed to load the shared library\n");
        return 1;
    }

    for (int i = 0; i < arguments.nb_functions; i++) {
        char** func = &arguments.functions[i];
        char* (*function)() = my_dlsym(handle, *func);
        if (!function) {
            dprintf(STDERR_FILENO, "Failed to find the %s function\n", *func);
            return 1;
        }
        if (arguments.verbose) {
            printf("%s() returned %s\n", *func, function());
        }
    }

    int fd = open(arguments.file, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return 1;
    }

    Elf64_Ehdr header;
    parse_elf_header(fd, &header);

    Elf64_Phdr *pheaders = NULL;
    int nb_seg = parse_program_headers(fd, &header, &pheaders);
    int total_size = compute_total_size(pheaders, nb_seg);

    void* base = load_segments(fd, pheaders, nb_seg, total_size);
    printf("Loaded ELF at base address: %p\n", base);

    close(fd);

    free(pheaders);

    return 0;
}
