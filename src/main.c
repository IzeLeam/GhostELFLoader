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
#include "map.h"

struct arguments_t arguments = {NULL, 0, 0, NULL};

void perform_relocations(void *base_addr, Elf64_Dyn *dynamic) {
    Elf64_Rela *rela = NULL;
    Elf64_Xword relasz = 0;
    
    // Loop through dynamic entries to find the relocation table and its size.
    for (Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; dyn++) {
         switch(dyn->d_tag) {
             case DT_RELA:
                 // DT_RELA contains an offset relative to the load base.
                 rela = (Elf64_Rela *)((char *)base_addr + dyn->d_un.d_ptr);
                 break;
             case DT_RELASZ:
                 relasz = dyn->d_un.d_val;
                 break;
             default:
                 break;
         }
    }
    
    if (!rela || relasz == 0) {
        printf("No relocations to process.\n");
        return;
    }
    
    size_t num_rela = relasz / sizeof(Elf64_Rela);
    printf("Processing %zu relocation entries...\n", num_rela);
    
    // Process each relocation entry.
    for (size_t i = 0; i < num_rela; i++) {
         Elf64_Rela *rel = &rela[i];
         
         // We only handle R_X86_64_RELATIVE relocations.
         if (ELF64_R_TYPE(rel->r_info) == R_X86_64_RELATIVE) {
              // Compute the address to patch:
              // The relocation offset is relative to the base load address.
              Elf64_Addr *addr_to_patch = (Elf64_Addr *)((char *)base_addr + rel->r_offset);
              
              // The new value is the base address plus the addend.
              *addr_to_patch = (Elf64_Addr)base_addr + rel->r_addend;
              
              // Optionally, print the relocation details for debugging.
              printf("Relocated [%zu]: target at %p set to 0x%lx\n",
                     i, (void *)addr_to_patch, *addr_to_patch);
         }
    }
}

int main(int argc, char **argv) {

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.file == NULL) {
        dprintf(STDERR_FILENO, "No file specified\n");
        return 1;
    }

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

    void* base_address = load_segments(fd, pheaders, nb_seg, total_size);

    Elf64_Dyn* dynamic = NULL;
    for (int i = 0; i < nb_seg; i++) {
        if (pheaders[i].p_type == PT_DYNAMIC) {
            // The p_vaddr field is relative to the load base.
            dynamic = (Elf64_Dyn *)((char *)base_address + pheaders[i].p_vaddr);
            break;
        }
    }
    if (!dynamic) {
        fprintf(stderr, "Failed to locate PT_DYNAMIC segment.\n");
        exit(1);
    }

    // Process the relocations using the dynamic table.
    perform_relocations(base_address, dynamic);

    close(fd);

    free(pheaders);

    return 0;
}
