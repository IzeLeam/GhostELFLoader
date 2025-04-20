#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include <string.h>

#include "elf.h"
#include "map.h"
#include "args_parser.h"

void print_segment_contents(Elf64_Phdr* pheaders, int nb_seg, void* base_address) {
    for (int i = 0; i < nb_seg; i++) {
        Elf64_Phdr* ph = &pheaders[i];

        // On ignore les segments vides
        if (ph->p_memsz == 0)
            continue;

        Elf64_Addr segment_vaddr = ph->p_vaddr + (Elf64_Addr)base_address;
        size_t segment_memsz = ph->p_memsz;

        printf("\nSegment %d (vaddr=0x%lx, size=0x%lx, real address=0x%lx):\n", 
               i, ph->p_vaddr, segment_memsz, segment_vaddr);

        // Lire le segment en mémoire et l'afficher en hexdump
        unsigned char* segment_data = (unsigned char*) segment_vaddr;
        for (size_t j = 0; j < segment_memsz; j += 16) {
            // Adresse relative
            printf("  %08lx  ", ph->p_vaddr + j);

            // Partie hexadécimale
            for (size_t k = 0; k < 16; k++) {
                if (j + k < segment_memsz)
                    printf("%02x ", segment_data[j + k]);
                else
                    printf("   ");
            }

            // Séparateur
            printf(" ");

            // Partie ASCII
            for (size_t k = 0; k < 16; k++) {
                if (j + k < segment_memsz) {
                    char c = segment_data[j + k];
                    printf("%c", isprint(c) ? c : '.');
                }
            }
            printf("\n");
        }
    }
}

void* load_segments(int fd, Elf64_Phdr* pheaders, int nb_seg, int total_size) {
    size_t page_size = sysconf(_SC_PAGESIZE);

    // Allocate memory for the segments
    Elf64_Addr base_address = (Elf64_Addr) mmap((void*)0x00, total_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ((void *)base_address == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Load every segment in the memory
    for (int i = 0; i < nb_seg; i++) {
        Elf64_Phdr* ph = &pheaders[i];

        Elf64_Addr segment_vaddr = ph->p_vaddr + base_address;
        size_t segment_filesz = ph->p_filesz;
        size_t segment_memsz = ph->p_memsz;

        // Align the segment address and offset
        size_t extra_offset = segment_vaddr % page_size;
        Elf64_Addr aligned_vaddr = segment_vaddr - extra_offset;
        off_t aligned_offset = ph->p_offset - (ph->p_offset % page_size);

        // Map the segment into memory
        void* mapped = mmap((void*)aligned_vaddr, segment_memsz + extra_offset,
                            PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, aligned_offset);
        if (mapped == MAP_FAILED) {
            perror("mmap segment failed");
            exit(1);
        }

        // Fill the extra offset with zero
        if (extra_offset > 0) {
            void* extra_start = (void*)aligned_vaddr;
            for (size_t j = 0; j < extra_offset; j++) {
                ((unsigned char*)extra_start)[j] = 0;
            }
        }

        // If the segment is the BSS, fill it with zero
        if (segment_memsz > segment_filesz) {
            void* bss_start = (void*)(segment_vaddr + segment_filesz);
            size_t bss_size = segment_memsz - segment_filesz;
            for (size_t j = 0; j < bss_size; j++) {
                ((unsigned char*)bss_start)[j] = 0;
            }
        }

        // Assemble the protection flags
        int prot = 0;
        if (ph->p_flags & PF_R) prot |= PROT_READ;
        if (ph->p_flags & PF_W) prot |= PROT_WRITE;
        if (ph->p_flags & PF_X) prot |= PROT_EXEC;

        // Set the protection on the segment
        if (mprotect((void*)aligned_vaddr, segment_memsz + extra_offset, prot) < 0) {
            perror("mprotect failed");
            exit(1);
        }
    }

    if (arguments.verbose) {
        printf("Segments loaded in memory:\n");
        print_segment_contents(pheaders, nb_seg, (void*)base_address);
    }

    return (void*)base_address;
}
