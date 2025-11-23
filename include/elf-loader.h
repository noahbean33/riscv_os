#pragma once
#include <stdint.h>
#include <stddef.h>

#define PT_LOAD   1
#define ELF_MAGIC 0x464C457F  // "\x7FELF" in little-endian

// ELF Program Header Flags
#define PF_X  (1 << 0)  // Execute
#define PF_W  (1 << 1)  // Write
#define PF_R  (1 << 2)  // Read

typedef struct {
    uint32_t e_magic;   // ELF_MAGIC
    uint8_t  e_elf[12]; // ELF header remainder (class, endian, version, etc.)
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef struct {
    uint32_t sh_name;       // Offset in the section header string table
    uint32_t sh_type;       // Sectietype (SHT_*)
    uint64_t sh_flags;      // Flags (SHF_*)
    uint64_t sh_addr;       // Virtueel address (if loaded)
    uint64_t sh_offset;     // Offset in the file
    uint64_t sh_size;       // Size of this section
    uint32_t sh_link;       // Index to another section (often strtab)
    uint32_t sh_info;       // Additional section information
    uint64_t sh_addralign;  // Alignment of section in memory/file
    uint64_t sh_entsize;    // Size of an entry if it is a table
} Elf64_Shdr;

typedef struct {
    uint32_t st_name;   // Offset to the name in the string table
    uint8_t  st_info;   // Type and binding of the symbol
    uint8_t  st_other;  // Ounused (almost always 0)
    uint16_t st_shndx;  // Section-index (SHN_*)
    uint64_t st_value;  // Address of offset of the symbool
    uint64_t st_size;   // Size of the symbol (in bytes)
} Elf64_Sym;

struct process* extract_flat_binary_from_elf(const void *elf_data, int create_process_flag);