#pragma once

#include <stdint.h>

#define EI_NIDENT   16
#define ELFCLASS64	2
#define ET_DYN		  3

#define EI_MAG0		0		/* File identification byte 0 index */
#define ELFMAG0		0x7f		/* Magic number byte 0 */
#define EI_MAG1		1		/* File identification byte 1 index */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define EI_MAG2		2		/* File identification byte 2 index */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define EI_MAG3		3		/* File identification byte 3 index */
#define ELFMAG3		'F'		/* Magic number byte 3 */

#define PT_LOAD     1

typedef __uint16_t Elf64_Half;
typedef __uint32_t Elf64_Word;
typedef __uint64_t Elf64_Addr;
typedef __uint64_t Elf64_Off;
typedef __uint64_t Elf64_Xword;

// ELF header
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

// Program header
typedef struct {
  Elf64_Word	  p_type;			  /* Segment type               */
  Elf64_Word	  p_flags;		  /* Segment flags              */
  Elf64_Off	    p_offset;		  /* Segment file offset        */
  Elf64_Addr	  p_vaddr;		  /* Segment virtual address    */
  Elf64_Addr	  p_paddr;		  /* Segment physical address   */
  Elf64_Xword	  p_filesz;		  /* Segment size in file       */
  Elf64_Xword	  p_memsz;		  /* Segment size in memory     */
  Elf64_Xword	  p_align;		  /* Segment alignment          */
} Elf64_Phdr;

void parse_elf_header(char* filename, Elf64_Ehdr* header);

void parse_program_headers(char* filename, Elf64_Ehdr* eheader, Elf64_Phdr** pheaders);
