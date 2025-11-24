#include "kernel.h"
#include "elf-loader.h"
#include "uart.h"
#include "process.h"
#include "page.h"
#include "string.h"

extern struct process *current_proc; // Currently running process

// Globally usable in create_process.c
uintptr_t g_user_stack_top = 0;
uintptr_t g_user_stack_bottom = 0;
uintptr_t g_user_heap_start = 0;
uintptr_t g_user_heap_end = 0;

uint64_t get_symbol_address(const void *elf_data, const char *symbol_name) {
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)elf_data;
    const Elf64_Shdr *shdrs = (const Elf64_Shdr *)((const uint8_t *)elf_data + ehdr->e_shoff);

    // Find the string table index (for symbol names)
    const Elf64_Shdr *sh_strtab = &shdrs[ehdr->e_shstrndx];
    const char *sh_strtab_p = (const char *)elf_data + sh_strtab->sh_offset;

    // Find .symtab and its corresponding .strtab
    const Elf64_Shdr *symtab = NULL;
    const Elf64_Shdr *strtab = NULL;

    for (int i = 0; i < ehdr->e_shnum; ++i) {
        const char *name = sh_strtab_p + shdrs[i].sh_name;

        if (strcmp(name, ".symtab") == 0)
            symtab = &shdrs[i];
        else if (strcmp(name, ".strtab") == 0)
            strtab = &shdrs[i];
    }

    if (!symtab || !strtab)
        PANIC("Missing .symtab or .strtab");

    const Elf64_Sym *symbols = (const Elf64_Sym *)((const uint8_t *)elf_data + symtab->sh_offset);
    size_t symbol_count = symtab->sh_size / sizeof(Elf64_Sym);
    const char *strtab_p = (const char *)elf_data + strtab->sh_offset;

    for (size_t i = 0; i < symbol_count; ++i) {
        const char *name = strtab_p + symbols[i].st_name;
        if (strcmp(name, symbol_name) == 0) {
            return symbols[i].st_value;
        }
    }

    PANIC("Symbol not found");
}

struct process* extract_flat_binary_from_elf(const void *elf_data, int create_process_flag) {
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *) elf_data;

    if (ehdr->e_magic != ELF_MAGIC)
        PANIC("Invalid ELF magic");

    const Elf64_Phdr *phdr = (const Elf64_Phdr *) ((const uint8_t *) elf_data + ehdr->e_phoff);

    // Determine total virtual space required (for alloc)
    uint64_t min_vaddr = (uint64_t)-1;
    uint64_t max_vaddr = 0;

    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        if (phdr[i].p_vaddr < min_vaddr)
            min_vaddr = phdr[i].p_vaddr;
        if (phdr[i].p_vaddr + phdr[i].p_memsz > max_vaddr)
            max_vaddr = phdr[i].p_vaddr + phdr[i].p_memsz;
    }

    // Round to page size
    min_vaddr &= ~(PAGE_SIZE - 1);
    max_vaddr = (max_vaddr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    size_t image_size = max_vaddr - min_vaddr;

    // Allocate flat memory buffer
    void *flat_image = (void *)(uintptr_t)alloc_pages((image_size + PAGE_SIZE - 1) / PAGE_SIZE);
    memset(flat_image, 0, image_size);

    // Copy segments into buffer
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        const void *src = (const uint8_t *) elf_data + phdr[i].p_offset;
        void *dest = (uint8_t *) flat_image + (phdr[i].p_vaddr - min_vaddr);

        memcpy(dest, src, phdr[i].p_filesz);
        // memsz > filesz: rest is already zeroed
    }

    g_user_stack_top = get_symbol_address(elf_data, "__user_stack_top");
    g_user_stack_bottom = get_symbol_address(elf_data, "__user_stack_bottom");
    g_user_heap_start = get_symbol_address(elf_data, "__user_heap_start");
    g_user_heap_end = get_symbol_address(elf_data, "__user_heap_end");

    // Creëer proces 
    if (create_process_flag)
        return create_init_process(flat_image, image_size, 1);
    else 
        return exec_process(flat_image, image_size, 0);
    
}