#include "main.h"
#include "screen.h"
#include "interrupt.h"
#include "timer.h"
#include "rtc.h"
#include "keyboard.h"
#include "console.h"
#include "ata.h"
#include "memory.h"

WORD* buf[1000];

int divideByZero(int param)
{
	printf("Devide\n");
	__magic();
	printf("Div  0 = %d\n", 10 / param);
	return param;
}

void KernelMain()
{
    __magic();    // break into BOCHS
    
    //__enableSSE();  // only for demo; in the future will be called from __init.asm
    ClearScreen();
	//__magic();
	__magic();
	HelloBoot();
	PIC_remap(0x20, 0x28);
	ClearScreen();
	InitIDT();
	TimerInit();
	RTCInit();
	KeyboardInit();
	IRQ_clear_mask(0);
	IRQ_clear_mask(1);
	IRQ_clear_mask(8);
	InitConsole();
	__sti();
	//detect_ata();
	//ATAIdentify(FIRST_DRIVE_PORT_BASE, ATA_MASTER);
	ide_initialize(	FIRST_DRIVE_PORT_BASE, 
					FIRST_DRIVE_CONTROL_BASE, 
					SECOND_DRIVE_PORT_BASE, 
					SECOND_DRIVE_CONTROL_BASE, 
					0x000);
	//ide_read_sectors(0, 1, 0, 0, 0);
	//ide_read_sectors(0, 1, 0, buf, 1000);
	//printf("bufAddr: %x\n", buf);
	//__magic();
	//__magic();	
	//divideByZero(0);
	InitMemory();
	//printf("BitMap addr: %x\n", &(BitMap.bits));
	/*PageAlloc(1);
	PageAlloc(1);
	PageAlloc(100);*/
	__magic();
	ClearScreen();
	QWORD* address0 = MemBlockAlloc(1);
	if (address0 != NULL)
		printf("Memory Address = %x\n", address0);
	else
		printf("NULL\n");
	__magic();
	//ClearScreen();
	QWORD* address1 = MemBlockAlloc(10);
	if (address1 != NULL)
		printf("Memory Address = %x\n", address1);
	else
		printf("NULL\n");
	__magic();
	//ClearScreen();
	QWORD* address2 = MemBlockAlloc(4200);
	if (address2 != NULL)
		printf("Memory Address = %x\n", address2);
	else
		printf("NULL\n");
	__magic();
	printf("BitMap[0] addr: %x\n", (BitMap.bits[0]));
	printf("free %x\n", address2);
	MemBlockFree(address2);
	printf("BitMap[0] after free: %x, addr: %x\n", address2, (BitMap.bits[0]));
	printf("free %x\n", address1);
	MemBlockFree(address1);
	printf("BitMap[0] after free: %x, addr: %x\n", address1, (BitMap.bits[0]));
	printf("free %x\n", address0);
	MemBlockFree(address0);
	printf("BitMap[0] after free: %x, addr: %x\n", address0, (BitMap.bits[0]));
	ExecuteConsole();


	while (1)
	{
		/*ClearScreen();
		printf("time = %d\n", SysGetTime());
		mSleep(100);
		i++;*/
	}

    // TODO!!! PIC programming; see http://www.osdever.net/tutorials/view/programming-the-pic
    // TODO!!! define interrupt routines and dump trap frame
    
    // TODO!!! Timer programming

    // TODO!!! Keyboard programming

    // TODO!!! Implement a simple console

    // TODO!!! read disk sectors using PIO mode ATA

    // TODO!!! Memory management: virtual, physical and heap memory allocators
}
