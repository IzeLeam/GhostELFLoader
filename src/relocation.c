#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "elf.h"
#include "args_parser.h"

void relocate_dynsym(void *base_addr, Elf64_Dyn *dynamic, Elf64_Phdr *pheaders, int nb_seg) {
    Elf64_Rela *rela = NULL;
    Elf64_Xword relasz = 0;

    // Locate the .rela.dyn section
    for (Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_RELA:
                rela = (Elf64_Rela *)((uintptr_t)base_addr + dyn->d_un.d_ptr);
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

    for (size_t i = 0; i < num_rela; i++) {
        Elf64_Rela *rel = &rela[i];

        if (ELF64_R_TYPE(rel->r_info) == R_X86_64_RELATIVE) {
            Elf64_Addr *addr_to_relocate = (Elf64_Addr *)((uintptr_t)base_addr + rel->r_offset);
            Elf64_Addr new_value = (Elf64_Addr)base_addr + rel->r_addend;

            // Check if the address is in a read-only segment
            int is_read_only = 0;
            for (int j = 0; j < nb_seg; j++) {
                Elf64_Phdr *ph = &pheaders[j];
                if (ph->p_type == PT_LOAD &&
                    (Elf64_Addr)addr_to_relocate >= (Elf64_Addr)base_addr + ph->p_vaddr &&
                    (Elf64_Addr)addr_to_relocate < (Elf64_Addr)base_addr + ph->p_vaddr + ph->p_memsz) {
                    if (!(ph->p_flags & PF_W)) {
                        is_read_only = 1;
                    }
                    break;
                }
            }

            // Temporarily make the page writable if it's in a read-only segment
            if (is_read_only) {
                void *page_start = (void *)((Elf64_Addr)addr_to_relocate & ~(sysconf(_SC_PAGESIZE) - 1));
                if (mprotect(page_start, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE) < 0) {
                    perror("mprotect failed");
                    exit(1);
                }
            }

            // Perform the relocation
            *addr_to_relocate = new_value;

            // Restore the original permissions if modified
            if (is_read_only) {
                void *page_start = (void *)((Elf64_Addr)addr_to_relocate & ~(sysconf(_SC_PAGESIZE) - 1));
                if (mprotect(page_start, sysconf(_SC_PAGESIZE), PROT_READ) < 0) {
                    perror("mprotect failed");
                    exit(1);
                }
            }

            if (arguments.verbose) {
                printf("Relocated [%zu]: target at %p set to 0x%lx\n", i, (void *)addr_to_relocate, *addr_to_relocate);
            }
        }
    }
}