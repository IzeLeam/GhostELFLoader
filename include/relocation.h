#pragma once

void find_dynamic_section(int fd, Elf64_Ehdr *header, void *base_address, Elf64_Dyn* buffer);

void relocate_dynsym(void *base_addr, Elf64_Dyn *dynamic, Elf64_Phdr *pheaders, int nb_seg);