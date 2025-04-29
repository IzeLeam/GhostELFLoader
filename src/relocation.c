#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#include "elf_parser.h"
#include "args_parser.h"

/**
 * Find the dynamic section in the ELF file
 * 
 * This function locates the PT_DYNAMIC segment in the ELF file and returns its address.
 * 
 * @param fd The file descriptor of the ELF file
 * @param header The ELF header
 * @param base_address The base address of the loaded ELF file
 * @param buffer A pointer to store the address of the dynamic section
 */
void find_dynamic_section(int fd, Elf64_Ehdr *header, void *base_address, Elf64_Dyn* buffer) {
    Elf64_Dyn *dynamic = NULL;
    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr ph;
        lseek(fd, header->e_phoff + i * sizeof(Elf64_Phdr), SEEK_SET);
        read(fd, &ph, sizeof(Elf64_Phdr));

        if (ph.p_type == PT_DYNAMIC) {
            dynamic = (Elf64_Dyn *)((uintptr_t)base_address + ph.p_vaddr);
            break;
        }
    }
    if (!dynamic) {
        dprintf(STDERR_FILENO, "Failed to locate PT_DYNAMIC segment.\n");
        return;
    }
    if (arguments.verbose) {
        printf("PT_DYNAMIC segment located at %p\n", dynamic);
    }
}

/**
 * Relocate the dynamic symbol table
 * 
 * This function processes the relocation entries in the .rela.dyn section of the ELF file.
 * Only R_X86_64_RELATIVE relocations are handled, which adjust the address of the symbol
 * 
 * @param base_addr The base address of the loaded ELF file
 * @param dynamic The dynamic section of the ELF file
 * @param pheaders The program headers of the ELF file
 * @param nb_seg The number of program headers
 */
void relocate_dynsym(void *base_addr, Elf64_Dyn *dynamic, Elf64_Phdr *pheaders, int nb_seg) {
    Elf64_Rela *rela = NULL;
    size_t num_rela = 0;

    // Get relocation informations in the dynamic section
    for (Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_RELA:
                rela = (Elf64_Rela *)((uintptr_t)base_addr + dyn->d_un.d_ptr);
                break;
            case DT_RELACOUNT:
                num_rela = dyn->d_un.d_val;
                break;
            default:
                break;
        }
    }

    // Assert the relocation exists
    if (!rela || num_rela == 0) {
        printf("No relocations to process.\n");
        return;
    } else if (arguments.verbose) {
        printf("%zu relocations to process.\n", num_rela);
    }

    for (size_t i = 0; i < num_rela; i++) {
        Elf64_Rela *rel = &rela[i];

        // Filter only relative relocations (R_X86_64_RELATIVE or R_AARCH64_RELATIVE)
        if (ELF64_R_TYPE(rel->r_info) == R_X86_64_RELATIVE || ELF64_R_TYPE(rel->r_info) == R_AARCH64_RELATIVE) {
            Elf64_Addr *addr_to_relocate = (Elf64_Addr *)((uintptr_t) base_addr + rel->r_offset);
            Elf64_Addr new_value = (Elf64_Addr)base_addr + rel->r_addend;
            
            // If in a read-only segment
            int is_read_only = 0;
            int prot = 0;

            for (int j = 0; j < nb_seg; j++) {
                Elf64_Phdr *ph = &pheaders[j];
                // Find the segment that contains the address to relocate
                if (ph->p_type == PT_LOAD &&
                    (Elf64_Addr)addr_to_relocate >= (Elf64_Addr)base_addr + ph->p_vaddr &&
                    (Elf64_Addr)addr_to_relocate < (Elf64_Addr)base_addr + ph->p_vaddr + ph->p_memsz) {
                    if (!(ph->p_flags & PF_W)) {
                        is_read_only = 1;
                        // Save the original permissions
                        if (ph->p_flags & PF_R) prot |= PROT_READ;
                        if (ph->p_flags & PF_W) prot |= PROT_WRITE;
                        if (ph->p_flags & PF_X) prot |= PROT_EXEC;
                        if (arguments.verbose) {
                            printf("Relocation in read-only segment: %p\n", addr_to_relocate);
                        }
                    }
                    break;
                }
            }

            // Set the read and write permissions on the page
            if (is_read_only) {
                void *aligned_addr = (void *)((Elf64_Addr)addr_to_relocate - ((Elf64_Addr)addr_to_relocate % sysconf(_SC_PAGESIZE)));
                if (mprotect(aligned_addr, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE) < 0) {
                    perror("mprotect failed");
                    exit(1);
                }
            }

            // Relocation
            *addr_to_relocate = new_value;
            
            // Restore permissions
            if (is_read_only) {
                void *aligned_addr = (void *)((Elf64_Addr)addr_to_relocate - ((Elf64_Addr)addr_to_relocate % sysconf(_SC_PAGESIZE)));
                if (mprotect(aligned_addr, sysconf(_SC_PAGESIZE), prot) < 0) {
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