#ifndef _MAIN_H_
#define _MAIN_H_

#include <intrin.h>

#define TRUE 1
#define FALSE 0

#define DEBUG TRUE

//
// default types
//
typedef unsigned __int8     BYTE, *PBYTE, BOOLEAN;
typedef unsigned __int16    WORD, *PWORD;
typedef unsigned __int32    DWORD, *PDWORD;
typedef unsigned __int64    QWORD, *PQWORD;
typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;

//
// exported functions from __init.asm
//
void __cli(void);
void __sti(void);
void __magic(void);         // MAGIC breakpoint into BOCHS (XCHG BX,BX)
void __enableSSE(void);
void __IRQ0(void);
void __IRQ1(void);
void __IRQ2(void);
void __IRQ3(void);
void __IRQ4(void);
void __IRQ5(void);
void __IRQ6(void);
void __IRQ7(void);
void __IRQ8(void);
void __IRQ9(void);
void __ERQ5(void);
void __ERQ8(void);
void __ERQ14(void);
WORD __get_intr_flags();
void __set_intr_flags(QWORD flags);

#endif // _MAIN_H_