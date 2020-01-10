#include "interrupt.h"
#include "screen.h"
#include "timer.h"
#include "keyboard.h"

static struct IDTDescr64 IDT[256];

void PIC_sendEOI(unsigned char irq)
{
	if (irq >= 8)
	{
		__outbyte(S_PIC_COMMAND_PORT, PIC_EOI);
	}
	__outbyte(M_PIC_COMMAND_PORT, PIC_EOI);
}

void PIC_remap(int offset1, int offset2)
{
	BYTE a1, a2;

	a1 = __inbyte(M_PIC_DATA_PORT);                        // save masks
	a2 = __inbyte(S_PIC_DATA_PORT);

	__outbyte(M_PIC_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	__outbyte(S_PIC_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);

	__outbyte(M_PIC_DATA_PORT, offset1);                 // ICW2: Master PIC vector offset
	__outbyte(S_PIC_DATA_PORT, offset2);                 // ICW2: Slave PIC vector offset

	__outbyte(M_PIC_DATA_PORT, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	__outbyte(S_PIC_DATA_PORT, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)

	__outbyte(M_PIC_DATA_PORT, ICW4_8086);
	__outbyte(S_PIC_DATA_PORT, ICW4_8086);

	__outbyte(M_PIC_DATA_PORT, 0xFF);
}

void IRQ_set_mask(unsigned char IRQline)
{
	WORD port;
	BYTE value;

	if (IRQline < 8) {
		port = M_PIC_DATA_PORT;
	}
	else {
		port = S_PIC_DATA_PORT;
		IRQline -= 8;
	}
	value = __inbyte(port) | (1 << IRQline);
	__outbyte(port, value);
}

void IRQ_clear_mask(unsigned char IRQline) {
	WORD port;
	BYTE value;

	if (IRQline < 8) {
		port = M_PIC_DATA_PORT;
	}
	else {
		port = S_PIC_DATA_PORT;
		IRQline -= 8;
	}
	value = __inbyte(port) & ~(1 << IRQline);
	__outbyte(port, value);
}

void InitIDT()
{
	QWORD irq0_address;
	QWORD irq1_address;
	QWORD irq2_address;
	QWORD irq3_address;
	QWORD irq4_address;
	QWORD irq5_address;
	QWORD irq6_address;
	QWORD irq7_address;
	QWORD irq8_address;
	QWORD irq9_address;
	QWORD irq10_address;
	QWORD irq11_address;
	QWORD irq12_address;
	QWORD irq13_address;
	QWORD irq14_address;
	QWORD irq15_address;
	QWORD idt_address;
	QWORD idt_ptr[2];

	QWORD exception0_address;
	QWORD exception5_address;
	QWORD exception8_address;
	QWORD exception14_address;

	PIC_remap(0x20, 0x28);

	// Division by zero exception
	exception0_address = (QWORD)__ERQ0;
	IDT[0].offset_1 = (exception0_address & 0xffffffff00000000) >> 32;
	IDT[0].offset_2 = (exception0_address & 0x00000000ffff0000) >> 16;
	IDT[0].type_attr = 0x8e;
	IDT[0].zero = 0;
	IDT[0].selector = 48;
	IDT[0].offset_3 = (exception0_address & 0xffff);

	// Bound range exceded exception
	exception5_address = (QWORD)__ERQ5;
	IDT[5].offset_1 = (exception5_address & 0xffffffff00000000) >> 32;
	IDT[5].offset_2 = (exception5_address & 0x00000000ffff0000) >> 16;
	IDT[5].type_attr = 0x8e;
	IDT[5].zero = 0;
	IDT[5].selector = 48;
	IDT[5].offset_3 = (exception5_address & 0xffff);

	// Double Fault exception
	exception8_address = (QWORD)__ERQ8;
	IDT[8].offset_1 = (exception8_address & 0xffffffff00000000) >> 32;
	IDT[8].offset_2 = (exception8_address & 0x00000000ffff0000) >> 16;
	IDT[8].type_attr = 0x8e;
	IDT[8].zero = 0;
	IDT[8].selector = 48;
	IDT[8].offset_3 = (exception8_address & 0xffff);

	// Page Fault exception
	exception14_address = (QWORD)__ERQ14;
	IDT[14].offset_1 = (exception14_address & 0xffffffff00000000) >> 32;
	IDT[14].offset_2 = (exception14_address & 0x00000000ffff0000) >> 16;
	IDT[14].type_attr = 0x8e;
	IDT[14].zero = 0;
	IDT[14].selector = 48;
	IDT[14].offset_3 = (exception14_address & 0xffff);

	// Timer interrupt
	irq0_address = (QWORD)__IRQ0;
	IDT[32].offset_1 = (irq0_address & 0xffffffff00000000) >> 32;
	IDT[32].offset_2 = (irq0_address & 0x00000000ffff0000) >> 16;
	IDT[32].type_attr = 0x8e;
	IDT[32].zero = 0;
	IDT[32].selector = 48;
	IDT[32].offset_3 = (irq0_address & 0xffff);

	// Keyboard interrupt
	irq1_address = (QWORD)__IRQ1;
	IDT[33].offset_1 = (irq1_address & 0xffffffff00000000) >> 32;
	IDT[33].offset_2 = (irq1_address & 0x00000000ffff0000) >> 16;
	IDT[33].type_attr = 0x8e;
	IDT[33].zero = 0;
	IDT[33].selector = 48;
	IDT[33].offset_3 = (irq1_address & 0xffff);

	irq2_address = (QWORD)__IRQ2;
	IDT[34].offset_1 = (irq2_address & 0xffffffff00000000) >> 32;
	IDT[34].offset_2 = (irq2_address & 0x00000000ffff0000) >> 16;
	IDT[34].type_attr = 0x8e;
	IDT[34].zero = 0;
	IDT[34].selector = 48;
	IDT[34].offset_3 = (irq2_address & 0xffff);

	irq3_address = (QWORD)__IRQ3;
	IDT[35].offset_1 = (irq3_address & 0xffffffff00000000) >> 32;
	IDT[35].offset_2 = (irq3_address & 0x00000000ffff0000) >> 16;
	IDT[35].type_attr = 0x8e;
	IDT[35].zero = 0;
	IDT[35].selector = 48;
	IDT[35].offset_3 = (irq3_address & 0xffff);

	irq4_address = (QWORD)__IRQ4;
	IDT[36].offset_1 = (irq4_address & 0xffffffff00000000) >> 32;
	IDT[36].offset_2 = (irq4_address & 0x00000000ffff0000) >> 16;
	IDT[36].type_attr = 0x8e;
	IDT[36].zero = 0;
	IDT[36].selector = 48;
	IDT[36].offset_3 = (irq4_address & 0xffff);

	irq5_address = (QWORD)__IRQ5;
	IDT[37].offset_1 = (irq5_address & 0xffffffff00000000) >> 32;
	IDT[37].offset_2 = (irq5_address & 0x00000000ffff0000) >> 16;
	IDT[37].type_attr = 0x8e;
	IDT[37].zero = 0;
	IDT[37].selector = 48;
	IDT[37].offset_3 = (irq5_address & 0xffff);

	irq6_address = (QWORD)__IRQ6;
	IDT[38].offset_1 = (irq6_address & 0xffffffff00000000) >> 32;
	IDT[38].offset_2 = (irq6_address & 0x00000000ffff0000) >> 16;
	IDT[38].type_attr = 0x8e;
	IDT[38].zero = 0;
	IDT[38].selector = 48;
	IDT[38].offset_3 = (irq6_address & 0xffff);

	irq7_address = (QWORD)__IRQ7;
	IDT[39].offset_1 = (irq7_address & 0xffffffff00000000) >> 32;
	IDT[39].offset_2 = (irq7_address & 0x00000000ffff0000) >> 16;
	IDT[39].type_attr = 0x8e;
	IDT[39].zero = 0;
	IDT[39].selector = 48;
	IDT[39].offset_3 = (irq7_address & 0xffff);

	// RTC interrupt
	irq8_address = (QWORD)__IRQ8;
	IDT[40].offset_1 = (irq8_address & 0xffffffff00000000) >> 32;
	IDT[40].offset_2 = (irq8_address & 0x00000000ffff0000) >> 16;
	IDT[40].type_attr = 0x8e;
	IDT[40].zero = 0;
	IDT[40].selector = 48;
	IDT[40].offset_3 = (irq8_address & 0xffff);

	idt_address = (QWORD)IDT;

	WORD address[5];
	address[0] = (sizeof(struct IDTDescr64) * 256);
	address[1] = idt_address & 0xffff;
	address[2] = (idt_address & 0xffff0000) >> 16;
	address[3] = (idt_address & 0xffff00000000) >> 32;
	address[4] = (idt_address & 0xffff000000000000) >> 48;

	__lidt(address);
}

WORD GetIntFlags()
{
	return __get_intr_flags();
}

WORD GetIntLevel()
{
	return GetIntFlags();
}

void SetInt(WORD intLvl)
{
	__set_intr_flags((QWORD)intLvl);
}

// TIMER INTERRUPT
void IRQ0_handler()
{
	//TimerInterruptHandler();
	TimerInterruptHandler();
	PIC_sendEOI(0);
}

// KEYBOARD INTERRUPT
void IRQ1_handler()
{
	KeyboardInterruptHandler();
	//debugPrint("IRQ1", 4);
	PIC_sendEOI(1);
}


void IRQ2_handler()
{
	debugPrint("IRQ2", 4);
	PIC_sendEOI(2);
}


void IRQ3_handler()
{
	debugPrint("IRQ3", 4);
	PIC_sendEOI(3);
}


void IRQ4_handler()
{
	debugPrint("IRQ4", 4);
	PIC_sendEOI(4);
}

void IRQ5_handler()
{
	debugPrint("IRQ5", 4);
	PIC_sendEOI(5);
}

void IRQ6_handler()
{
	debugPrint("IRQ6", 4);
	PIC_sendEOI(6);
}

void IRQ7_handler()
{
	debugPrint("IRQ7", 4);
	PIC_sendEOI(7);
}

// RTC INTERRUPT
void IRQ8_handler()
{
	debugPrint("IRQ8", 4);
	__outbyte(0x70, 0x0C);	// select register C
	__inbyte(0x71);		// just throw away contents
	PIC_sendEOI(8);
}
void IRQ9_handler()
{
	debugPrint("IRQ9", 4);
	PIC_sendEOI(9);
}

// DIVISION BY ZERO EXCEPTION
void ERQ0_handler()
{
	TRAP_FRAME* t;
	t = (TRAP_FRAME*)(__trap_esp()+(QWORD)8);
	t->error = 0;
	/*printf("esp = %d\n", t);
	printf("rip: %d, cs: %d, eflags: %x, esp: %d, ss: %d\n", t->rip, t->cs, t->eflags, t->esp, t->ss);*/
	PrintTrapFrame(t);
	debugPrint("Division By zero", 17);
	__cli();
	__hlt();
}

// BOUND RANGE EXCEEDED EXCEPTION
void ERQ5_handler()
{
	debugPrint("Bound Range Exceeded", 20);
}

// DOUBLE FAULT EXCEPTION
void ERQ8_handler()
{
	debugPrint("Double Fault", 12);
}

// PAGE FAULT EXCEPTION
void ERQ14_handler()
{
	TRAP_FRAME* t;
	t = (TRAP_FRAME*)(__trap_esp()+(QWORD)16);
	/*printf("esp = %d\n", t);
	printf("rip: %d, cs: %d, eflags: %x, esp: %d, ss: %d\n", t->rip, t->cs, t->eflags, t->esp, t->ss);*/
	PrintTrapFrame(t);
	debugPrint("Page Fault", 10);
	__cli();
	__hlt();
}

void PrintTrapFrame(TRAP_FRAME* Trap)
{
	printf("+------ TRAP FRAME ---------+ \n\
|    error:  %d \n\
|    rip:    %x \n\
|    cs:     %d \n\
|    eflags: %x \n\
|    esp:    %x \n\
|    ss:     %d \n\
+---------------------------+ \n",
		Trap->error, Trap->rip, Trap->cs, Trap->eflags, Trap->esp, Trap->ss);
}