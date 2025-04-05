#pragma once

void* load_segments(int fd, Elf64_Phdr* pheaders, int nb_seg, int total_size);