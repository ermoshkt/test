#ifndef JOS_INC_TRAP_H
#define JOS_INC_TRAP_H

// Trap numbers
#define T_DIVIDE         0  // Divide error
#define T_DEBUG          1  // Debug exception
#define T_NMI            2  // Non-maskable interrupt
#define T_BRKPT          3  // Breakpoint
#define T_OFLOW          4  // Overflow
#define T_BOUND          5  // Bound range exceeded
#define T_ILLOP          6  // Illegal opcode
#define T_DEVICE         7  // Device not available
#define T_DBLFLT         8  // Double fault
/* #define T_COPROC      9 */  // Reserved (not used since 486)
#define T_TSS           10  // Invalid TSS
#define T_SEGNP         11  // Segment not present
#define T_STACK         12  // Stack exception
#define T_GPFLT         13  // General protection fault
#define T_PGFLT         14  // Page fault
/* #define T_RES        15 */  // Reserved
#define T_FPERR         16  // Floating point error
#define T_ALIGN         17  // Aligment check
#define T_MCHK          18  // Machine check
#define T_SIMDERR       19  // SIMD floating point error

#define T_SYSCALL       48  // System call

#define T_DEFAULT      500  // catchall

#ifndef __ASSEMBLER__

#include <inc/types.h>

struct PushRegs {
    uint32_t reg_edi;
    uint32_t reg_esi;
    uint32_t reg_ebp;
    uint32_t reg_oesp;  // Useless padding
    uint32_t reg_ebx;
    uint32_t reg_edx;
    uint32_t reg_ecx;
    uint32_t reg_eax;
} __attribute__((packed));

struct Trapframe {
    struct PushRegs tf_regs;
    uint16_t tf_es;
    uint16_t tf_padding1;
    uint16_t tf_ds;
    uint16_t tf_padding2;
    uint32_t tf_trapno;
    /* below here defined by x86 hardware */
    uint32_t tf_err;
    uintptr_t tf_eip;
    uint16_t tf_cs;
    uint16_t tf_padding3;
    uint32_t tf_eflags;
    /* below here only when crossing rings, such as from user to kernel */
    uintptr_t tf_esp;
    uint16_t tf_ss;
    uint16_t tf_padding4;
} __attribute__((packed));

#endif /* !__ASSEMBLER__ */

#endif /* !JOS_INC_TRAP_H */
