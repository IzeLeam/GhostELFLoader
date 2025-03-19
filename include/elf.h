#pragma once

#include <stdint.h>

#define EI_NIDENT 16

typedef __uint16_t Elf64_Half;
typedef __uint32_t Elf64_Word;
typedef __uint64_t Elf64_Addr;
typedef __uint64_t Elf64_Off;

typedef struct {
    unsigned char   e_ident[EI_NIDENT]; /* Magic number and other info          */
    Elf64_Half      e_type;             /* Object file type                     */
    Elf64_Half      e_machine;          /* Architecture                         */
    Elf64_Word      e_version;          /* Object file version                  */
    Elf64_Addr      e_entry;            /* Entry point virtual address          */
    Elf64_Off       e_phoff;            /* Program header table file offset     */
    Elf64_Off       e_shoff;            /* Section header table file offset     */
    Elf64_Word      e_flags;            /* Processor-specific flags             */
    Elf64_Half      e_ehsize;           /* ELF header size in bytes             */
    Elf64_Half      e_phentsize;        /* Program header table entry size      */
    Elf64_Half      e_phnum;            /* Program header table entry count     */
    Elf64_Half      e_shentsize;        /* Section header table entry size      */
    Elf64_Half      e_shnum;            /* Section header table entry count     */
    Elf64_Half      e_shstrndx;         /* Section header string table index    */
} Elf64_Ehdr;

void print_elf_header(Elf64_Ehdr* header);

void parse_elf_header(char* filename, Elf64_Ehdr* header);

void check_elf_header(Elf64_Ehdr* header);