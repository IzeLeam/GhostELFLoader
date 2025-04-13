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
#define PT_DYNAMIC  2

#define DT_NULL		0		/* Marks end of dynamic array */
#define DT_RELA		7		/* Address of Rela relocs */
#define DT_RELASZ	8		/* Total size of Rela relocs */

#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_W		(1 << 1)	/* Segment is writable */
#define PF_R		(1 << 2)	/* Segment is readable */

#define R_X86_64_RELATIVE	8	/* Adjust by program base */
#define ELF64_R_TYPE(i)			((i) & 0xffffffff)

typedef __uint16_t Elf64_Half;
typedef __uint32_t Elf64_Word;
typedef __uint64_t Elf64_Addr;
typedef __uint64_t Elf64_Off;
typedef __uint64_t Elf64_Xword;
typedef __int64_t Elf64_Sxword;

typedef struct
{
  Elf64_Sxword	d_tag;			/* Dynamic entry type */
  union
    {
      Elf64_Xword d_val;		/* Integer value */
      Elf64_Addr d_ptr;			/* Address value */
    } d_un;
} Elf64_Dyn;

typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  Elf64_Xword	r_info;			/* Relocation type and symbol index */
  Elf64_Sxword	r_addend;		/* Addend */
} Elf64_Rela;

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

void parse_elf_header(int fd, Elf64_Ehdr* header);

int compute_total_size(Elf64_Phdr* pheaders, int nb_seg);

int parse_program_headers(int fd, Elf64_Ehdr* eheader, Elf64_Phdr** pheaders);
