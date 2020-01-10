#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "main/main.h"

#define M_PIC				0x20	/* IO base address for master PIC */
#define S_PIC				0xA0	/* IO base address for slave PIC */
#define M_PIC_COMMAND_PORT	0x20
#define M_PIC_DATA_PORT		0x21
#define S_PIC_COMMAND_PORT	0xA0
#define S_PIC_DATA_PORT	0xA1

#define PIC_EOI				0x20

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4		0x01
#define ICW1_SINGLE		0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
#define ICW1_INIT		0x10		/* Initialization - required! */

#define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO		0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM		0x10		/* Special fully nested (not) */

#pragma pack(push)
#pragma pack(1)
struct IDTDescr64
{
	WORD	offset_3;	/* 15..0 */
	WORD	selector;
	BYTE	zero;
	BYTE	type_attr;
	WORD	offset_2;	/* 31..16 */
	DWORD	offset_1;	/* 63..32 */
	DWORD	reserved;
};

typedef struct _TRAP_FRAME
{
	QWORD	error;
	QWORD	rip;
	QWORD	cs;
	QWORD	eflags;
	QWORD	esp;
	QWORD	ss;
}TRAP_FRAME;
#pragma pack(pop)

void IRQ_set_mask(unsigned char IRQline);
void IRQ_clear_mask(unsigned char IRQline);
void PIC_sendEOI(unsigned char irq);
void PIC_remap(int offset1, int offset2);
void InitIDT();
WORD GetIntFlags();
WORD GetIntLevel();
void SetInt(WORD intLvl);
void PrintTrapFrame(TRAP_FRAME* Trap);

void IRQ0_handler();
void IRQ1_handler();
void IRQ2_handler();
void IRQ3_handler();
void IRQ4_handler();
void IRQ5_handler();
void IRQ6_handler();
void IRQ7_handler();
void IRQ8_handler();
void IRQ9_handler();

#endif	// _INTERRPUT_H_