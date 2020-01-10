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

void KernelMain()
{
    //__magic();    // break into BOCHS
    
    //__enableSSE();  // only for demo; in the future will be called from __init.asm
    ClearScreen();
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
	ide_initialize(	FIRST_DRIVE_PORT_BASE, 
					FIRST_DRIVE_CONTROL_BASE, 
					SECOND_DRIVE_PORT_BASE, 
					SECOND_DRIVE_CONTROL_BASE, 
					0x000);
	InitMemory();
	ClearScreen();
	//QWORD* address0 = MemBlockAlloc(1);
	//if (address0 != NULL)
	//	printf("Memory Address = %x\n", address0);
	//else
	//	printf("NULL\n");
	////__magic();
	////ClearScreen();
	//QWORD* address1 = MemBlockAlloc(10);
	//if (address1 != NULL)
	//	printf("Memory Address = %x\n", address1);
	//else
	//	printf("NULL\n");
	////__magic();
	////ClearScreen();
	//QWORD* address2 = MemBlockAlloc(4200);
	//if (address2 != NULL)
	//	printf("Memory Address = %x\n", address2);
	//else
	//	printf("NULL\n");
	//(int)(*(address2 + 10)) = 10;
	//printf("address %d\n", (int)(*(address2 + 10)));
	/*__magic();
	printf("BitMap[0] addr: %x\n", (BitMap.bits[0]));
	printf("free %x\n", address2);
	MemBlockFree(address2);
	printf("BitMap[0] after free: %x, addr: %x\n", address2, (BitMap.bits[0]));
	printf("free %x\n", address1);
	MemBlockFree(address1);
	printf("BitMap[0] after free: %x, addr: %x\n", address1, (BitMap.bits[0]));
	printf("free %x\n", address0);
	MemBlockFree(address0);
	printf("BitMap[0] after free: %x, addr: %x\n", address0, (BitMap.bits[0]));*/
	ClearScreen();
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
