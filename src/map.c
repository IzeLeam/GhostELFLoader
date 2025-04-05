#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "elf.h"
#include "map.h"

void* load_segments(int fd, Elf64_Phdr* pheaders, int nb_seg, int total_size) {
    size_t page_size = sysconf(_SC_PAGESIZE);

    // Calculate the bounds of all load segments
    Elf64_Addr min_vaddr = (Elf64_Addr)-1;
    Elf64_Addr max_vaddr = 0;
    for (int i = 0; i < nb_seg; i++) {
        if (pheaders[i].p_vaddr < min_vaddr)
            min_vaddr = pheaders[i].p_vaddr;
        if (pheaders[i].p_vaddr + pheaders[i].p_memsz > max_vaddr)
            max_vaddr = pheaders[i].p_vaddr + pheaders[i].p_memsz;
    }

    // Align the inferior bound on page size
    Elf64_Addr aligned_min_vaddr = min_vaddr - (min_vaddr % page_size);

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
        Elf64_Addr aligned_vaddr = segment_vaddr - (segment_vaddr % page_size);
        off_t aligned_offset = ph->p_offset - (ph->p_offset % page_size);
        size_t extra_offset = segment_vaddr % page_size;

        // Map the segment into memory
        void* mapped = mmap((void*)aligned_vaddr, segment_filesz + extra_offset,
                            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, aligned_offset);
        if (mapped == MAP_FAILED) {
            perror("mmap segment failed");
            exit(1);
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

    return (void*)base_address;
}
