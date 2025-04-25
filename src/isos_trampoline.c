#include "isos-support.h"

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
 * @param handler  : the loader handler returned by my_dlopen().
 * @param import_id: the identifier of the function to be called
 *                   from the imported symbol table.
 * @return the address of the function to be called by the trampoline.
*/
void * loader_plt_resolver(void * handler, int import_id) {
	/* TODO */
}

