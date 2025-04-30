#pragma once

void relocate_dynsym(void *base_addr, Elf64_Dyn *dynamic, Elf64_Phdr *pheaders, int nb_seg);