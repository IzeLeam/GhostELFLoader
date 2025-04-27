
#pragma once

/**
 * Define some macros according to the host architecture.  
 */

#if defined(__x86_64__)
#include "arch/x86_64.h"
#elif defined(__arm__)
#include "arch/arm.h"
#elif defined(__aarch64__)
#include "arch/aarch64.h"
#else
#error "Unsupported architecture"
#endif


#define STR(...)  #__VA_ARGS__
#define XSTR(...) STR(__VA_ARGS__)
#define PUSH(x,y)        XSTR(_PUSH(x,y))
#define PUSH_IMM(x)      XSTR(_PUSH_IMM(x))
#define PUSH_STACK_STATE XSTR(_PUSH_STACK_STATE)
#define JMP_S(x)         XSTR(_JMP_S(x))
#define JMP_REG(x)       XSTR(_JMP_REG(x))
#define JMP(x,y)         XSTR(_JMP(x,y))
#define POP_S(x)         XSTR(_POP_S(x))
#define POP_STACK_STATE  XSTR(_POP_STACK_STATE)
#define CALL(x)          XSTR(_CALL(x))



/**
 * Define some macros for the custom PLT section.  
 */ 

#define PLT_BEGIN                                            \
    asm(".pushsection .text,\"ax\", \"progbits\""       "\n" \
        "plt_loader_path:"                              "\n" \
        PUSH(loader_handle, REG_IP)                     "\n" \
        JMP(isos_trampoline, REG_IP)                    "\n" \
        ".popsection"                                   "\n");

#define PLT_ENTRY(number, name)                          \
    asm(".pushsection .text,\"ax\", \"progbits\""       "\n" \
        #name ":"                                       "\n" \
        PUSH_IMM(number)                                "\n" \
        JMP_S(plt_loader_path)                          "\n" \
        ".popsection"                                   "\n");



/**
 * Define some macros for Bonus #1. 
 */ 

extern void * pltgot_entries[];
#define BONUS_PLT_BEGIN                                        \
    asm(".pushsection .text,\"ax\", \"progbits\""       "\n" \
        "plt_loader_path:"                              "\n" \
        PUSH(loader_handle, REG_IP)                     "\n" \
        JMP(isos_trampoline, REG_IP)                    "\n" \
        ".popsection" /* start of PLTGOT table. */      "\n" \
        ".pushsection .my_pltgot,\"aw\",\"progbits\""   "\n" \
        "pltgot_entries:"                               "\n" \
        ".popsection"                                   "\n");

#define BONUS_PLT_ENTRY(number, name)                          \
    asm(".pushsection .text,\"ax\", \"progbits\""       "\n" \
        #name ":"                                       "\n" \
        JMP(pltgot_ ##name, REG_IP)                     "\n" \
        "loadpath_" #name ":"                           "\n" \
        PUSH_IMM(number)                                "\n" \
        JMP_S(plt_loader_path)                          "\n" \
        ".popsection" /* entry in PLTGOT table */       "\n" \
        ".pushsection .my_pltgot,\"aw\",\"progbits\""   "\n" \
        "pltgot_" #name ":"                             "\n" \
        "." SYS_ADDR_ATTR " loadpath_" #name            "\n" \
        ".popsection"                                   "\n");
        