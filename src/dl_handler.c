#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "crypto.h"
#include "elf_parser.h"
#include "isos-support.h"
#include "relocation.h"
#include "segment_loader.h"
#include "lib.h"

/**
 * Debug function to print the loader entry
 * 
 * @param entry The loader entry to print
 */
static void debug_loader_entry(const loader_entry_t* entry) {
    if (!entry) {
        debug("Loader not found\n");
        return;
    }

    debug("\nLoader entry:\n");

    debug("  Exported symbols :\n");
    if (entry->exported) {
        for (int i = 0; entry->exported[i].name != NULL; i++) {
            debug("    Name: %s, Addr: %p\n", entry->exported[i].name, entry->exported[i].addr);
        }
    } else {
        debug("    NULL\n");
    }

    debug("  Imported symbols:\n");
    if (entry->imported) {
        for (int i = 0; entry->imported[i] != NULL; i++) {
            debug("    %s\n", entry->imported[i]);
        }
    } else {
        debug("    NULL\n");
    }

    debug("  PLT Table:\n");
    if (entry->plt_table) {
        for (int i = 0; entry->plt_table[i].name != NULL; i++) {
            debug("    Name: %s, Addr: %p\n", entry->plt_table[i].name, entry->plt_table[i].addr);
        }
    } else {
        debug("    NULL\n");
    }

    debug("  Trampoline address:\n");
    if (entry->trampoline) {
        debug("    %p\n", entry->trampoline);
    } else {
        debug("    NULL\n");
    }

    debug("  Handle address:\n");
    if (entry->handle) {
        debug("    %p\n", entry->handle);
    } else {
        debug("    NULL\n");
    }
}

/**
 * @brief The function isos_trampoline() is called by the PLT section
 * entry for each imported symbol inside the DL library.
*/
void isos_trampoline();
asm(".pushsection .text,\"ax\",\"progbits\""  "\n"
    "isos_trampoline:"                        "\n"
    POP_S(REG_ARG_1)                          "\n"
    POP_S(REG_ARG_2)                          "\n"
    PUSH_STACK_STATE                          "\n"
    CALL(loader_plt_resolver)                 "\n"
    POP_STACK_STATE                           "\n"
    JMP_REG(REG_RET)                          "\n"
    ".popsection"                             "\n");

/**
 * Implementation of the dlopen function
 * 
 * @param name The name of the shared library to load
 * 
 * @return A pointer to the loaded library in the memory, or NULL on failure
 */
void* my_dlopen(char* name, char* key) {

    // Open the file depending if the file is encrypted or not
    int fd;
    if (!key) {
        fd = open(name, O_RDONLY);
        if (fd < 0) {
            perror("Failed to open file");
            return NULL;
        }
    } else {
        fd = decrypt_library(name, key);
    }

    // ELF header parsing
    Elf64_Ehdr header;
    parse_elf_header(fd, &header);

    // Program header parsing
    Elf64_Phdr *pheaders = NULL;
    int nb_seg = parse_program_headers(fd, &header, &pheaders);
    int total_size = compute_total_size(pheaders, nb_seg);

    // Load the segments into memory
    void* base_address = load_segments(fd, pheaders, nb_seg, total_size);

    // Find the dynamic section
    Elf64_Dyn *dynamic = NULL;
    for (int i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr ph;
        lseek(fd, header.e_phoff + i * sizeof(Elf64_Phdr), SEEK_SET);
        read(fd, &ph, sizeof(Elf64_Phdr));

        if (ph.p_type == PT_DYNAMIC) {
            dynamic = (Elf64_Dyn *)((uintptr_t)base_address + ph.p_vaddr);
            break;
        }
    }
    if (!dynamic) {
        dprintf(STDERR_FILENO, "Failed to locate PT_DYNAMIC segment.\n");
        close(fd);
        free(pheaders);
        return NULL;
    }
    
    // Relocate the dynamic symbol table
    relocate_dynsym(base_address, dynamic, pheaders, nb_seg);

    // Get the handler address by using the entry point
    uint64_t *entry_dynsym = (uint64_t *)((uintptr_t) base_address + (header.e_entry - pheaders[0].p_vaddr));
    loader_entry_t *entry = (loader_entry_t *)*entry_dynsym;

    close(fd);
    free(pheaders);

    return entry;
}

/**
 * Implementation of the dlsym function
 * 
 * @param handle The address of the shared library returned by dlopen
 * @param func The name of the function to find
 * 
 * @return A pointer to the function, or NULL if not found
 */
void* my_dlsym(void* handle, char* func) {
    loader_entry_t *entry = (loader_entry_t *)handle;
    symbol_entry_t *tab = entry->exported;

    if (!tab) {
        return NULL;
    }

    // Return the address of the function by comparing the name
    for (int i = 0; tab[i].name != NULL; i++) {
        if (strcmp(tab[i].name, func) == 0) {
            return tab[i].addr;
        }
    }
    return NULL;
}

/**
 * Link the the loader entry with functons and PLT table
 * 
 * @brief This function links the imported symbols with the PLT table and
 * link both trampoline and the handle for the shared library to be able
 * to call them for the resolving.
 * 
 * @param handler The address of the shared library returned by dlopen
 * @param imported_symbols The imported symbols to resolve
 */
void my_dlset_plt_resolve(void* handler, symbol_entry_t imported_symbols[]) {
    if (!handler) {
        return;
    }

    loader_entry_t *tab = (loader_entry_t *)handler;

    // Save the imported symbols in the loader entry
    tab->plt_table = imported_symbols;

    // Load function addresses for the shared librarys
    *(tab->trampoline) = (void (*))isos_trampoline;
    *(tab->handle) = handler;

    debug_loader_entry(tab);
}

/**
 * Look for the imported symbol in the imported table by the ID
 * 
 * @param handler  : the loader handler returned by my_dlopen().
 * @param import_id: the identifier of the function to be called.
 * 
 * @return the name of the symbol associated with the ID.
*/
static const char* resolve_imported_symbol_by_id(void* handler, int import_id) {
    loader_entry_t *tab = (loader_entry_t *)handler;
    const char **imported_symbols = tab->imported;

    // Check if the ID is valid
    int imported_count = 0;
    while (imported_symbols[imported_count] != NULL) {
        imported_count++;
    }
    if (import_id < 0 || import_id >= imported_count) {
        return NULL;
    }

    return imported_symbols[import_id];
}

/**
 * Resolve the address of a function in the PLT table by its name
 * 
 * @param handler: the loader handler returned by my_dlopen().
 * @param name    : the name of the function.
 * 
 * @return the address of the function associated with the name.
*/
static void* resolve_function_in_plt(void* handler, const char* name) {
    loader_entry_t *tab = (loader_entry_t *)handler;
    symbol_entry_t *plt = tab->plt_table;

    if (!plt) {
        return NULL;
    }

    for (int i = 0; plt[i].name != NULL; i++) {
        if (strcmp(plt[i].name, name) == 0) {
            return plt[i].addr;
        }
    }

    return NULL;
}

/**
 * Resolve the address of a function in the PLT table by its ID
 * and cache it in the GOT table
 * 
 * @param handler  : the loader handler returned by my_dlopen().
 * @param import_id: the identifier of the function to be called
 *                   from the imported symbol table.
 * @return the address of the function to be called by the trampoline.
*/
void* loader_plt_resolver(void *handler, int import_id) {
    debug("Resolving imported symbol with ID: %d\n", import_id);

    loader_entry_t *tab = (loader_entry_t *)handler;

    // Resolve the symbol name
    const char *imported_symbol = resolve_imported_symbol_by_id(handler, import_id);
    if (!imported_symbol) {
        return NULL;
    }

    // Resolve the symbol address
    void *imported_addr = resolve_function_in_plt(handler, imported_symbol);
    if (!imported_addr) {
        return NULL;
    }

    // Cache the resolved address in the GOT table
    void** pltgot_entries = tab->pltgot_entries;
    pltgot_entries[import_id] = imported_addr;

    return imported_addr;
}
