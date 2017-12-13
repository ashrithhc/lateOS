#ifndef _ELF64_H
#define _ELF64_H

#define EI_NIDENT 16

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Lword;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Sxword
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half    e_type; // Object File Type
  Elf64_Half    e_machine; // Machine Type
  Elf64_Word    e_version; // Object File Version
  Elf64_Addr    e_entry; // Virtual adress entry point
  Elf64_Off     e_phoff; // Program header table offset
  Elf64_Off     e_shoff; // section header table offset
  Elf64_Word    e_flags; // processor-specific flags
  Elf64_Half    e_ehsize; // ELF HEader size in bytes
  Elf64_Half    e_phentsize; // size of program header entry
  Elf64_Half    e_phnum; // # of program header entries
  Elf64_Half    e_shentsize; //Size of section header entry
  Elf64_Half    e_shnum; // # of section header entries
  Elf64_Half    e_shstrndx; // Section name string table index
} Elf64_Ehdr;

typedef struct {
  Elf64_Word    p_type;  // Type of segment
  Elf64_Word    p_flags; // Segment attributes
  Elf64_Off     p_offset; // File offset
  Elf64_Addr    p_vaddr;  // VMA in memory
  Elf64_Addr    p_paddr;  // physical addr
  Elf64_Xword   p_filesz; // segment  size in file
  Elf64_Xword   p_memsz;  // segment size in memory
  Elf64_Xword   p_align; 
} Elf64_Phdr;

#endif
