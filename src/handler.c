#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "handler.h"
#include "args_parser.h"
#include "elf.h"
#include "map.h"
#include "relocation.h"

void* my_dlopen(char* name) {
    int fd = open(arguments.file, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return NULL;
    }

    Elf64_Ehdr header;
    parse_elf_header(fd, &header);

    Elf64_Phdr *pheaders = NULL;
    int nb_seg = parse_program_headers(fd, &header, &pheaders);
    int total_size = compute_total_size(pheaders, nb_seg);

    void* base_address = load_segments(fd, pheaders, nb_seg, total_size);

    Elf64_Dyn *dynamic = NULL;
    for (int i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr ph;
        lseek(fd, header.e_phoff + i * sizeof(Elf64_Phdr), SEEK_SET);
        read(fd, &ph, sizeof(Elf64_Phdr));

        if (ph.p_type == PT_DYNAMIC) {
            dynamic = (Elf64_Dyn *)((uintptr_t)base_address + ph.p_vaddr);
            break;
        }
    }
    if (!dynamic) {
        dprintf(STDERR_FILENO, "Failed to locate PT_DYNAMIC segment.\n");
        close(fd);
        free(pheaders);
        return NULL;
    }
    if (arguments.verbose) {
        printf("PT_DYNAMIC segment located at %p\n", dynamic);
    }

    relocate_dynsym(base_address, dynamic, pheaders, nb_seg);

    uint64_t *entry_dynsym = (uint64_t *)((uintptr_t)base_address + (header.e_entry - pheaders[0].p_vaddr));
    my_symbol_t *symbols = (my_symbol_t *)*entry_dynsym;

    if (arguments.verbose) {
        printf("Symbols located at %p\n", symbols);
    }

    // Print the symbols
    for (int i = 0; symbols[i].name != NULL; i++) {
        printf("Symbol: %s at %p\n", symbols[i].name, symbols[i].addr);
    }

    close(fd);

    free(pheaders);

    return symbols;
}

void* my_dlsym(void* handle, char* func) {
    my_symbol_t *tab = (my_symbol_t *)handle;
    if (!tab) {
        return NULL;
    }
    for (int i = 0; tab[i].name != NULL; i++) {
        if (strcmp(tab[i].name, func) == 0) {
            return tab[i].addr;
        }
    }
    return NULL;
}