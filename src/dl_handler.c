#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args_parser.h"
#include "dl_handler.h"
#include "elf_parser.h"
#include "isos-support.h"
#include "relocation.h"
#include "segment_loader.h"

/**
 * Implementation of the dlopen function
 * 
 * @param name The name of the shared library to load
 * 
 * @return A pointer to the loaded library in the memory, or NULL on failure
 */
void* my_dlopen(char* name) {
    int fd = open(name, O_RDONLY);
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

    uint64_t *entry_dynsym = (uint64_t *)((uintptr_t) base_address + (header.e_entry - pheaders[0].p_vaddr));
    loader_entry_t *entry = (loader_entry_t *)*entry_dynsym;

    exported_table_t *exported_symbols = entry->exported;
    const char **imported_symbols = entry->imported;

    if (arguments.verbose) {
        printf("Symbols located at %p\n", entry);
        printf("Exported symbols:\n");
        for (int i = 0; exported_symbols[i].name != NULL; i++) {
            printf("\t- %s at %p\n", exported_symbols[i].name, exported_symbols[i].addr);
        }
        printf("Imported symbols:\n");
        for (int i = 0; imported_symbols[i] != NULL; i++) {
            printf("\t- %s\n", imported_symbols[i]);
        }
    }

    close(fd);

    free(pheaders);

    return entry;
}

/**
 * Implementation of the dlsym function
 * 
 * @param handle The address of the shared library returned by dlopen
 * @param func The name of the function to find
 * 
 * @return A pointer to the function, or NULL if not found
 */
void* my_dlsym(void* handle, char* func) {
    loader_entry_t *entry = (loader_entry_t *)handle;
    exported_table_t *tab = entry->exported;

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

/**
 * @brief The function isos_trampoline() is called by the PLT section
 * entry for each imported symbol inside the DL library.
*/
void isos_trampoline();
asm(".pushsection .text,\"ax\",\"progbits\""  "\n"
    "isos_trampoline:"                        "\n"
    POP_S(REG_ARG_1)                          "\n"
    POP_S(REG_ARG_2)                          "\n"
    PUSH_STACK_STATE                          "\n"
    CALL(loader_plt_resolver)                 "\n"
    POP_STACK_STATE                           "\n"
    JMP_REG(REG_RET)                          "\n"
    ".popsection"                             "\n");


/**
 * @param handler  : the loader handler returned by my_dlopen().
 * @param import_id: the identifier of the function to be called
 *                   from the imported symbol table.
 * @return the address of the function to be called by the trampoline.
*/
void* loader_plt_resolver(void *handler, int import_id) {
    // TODO
}
