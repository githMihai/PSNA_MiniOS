#include "main.h"
#include "screen.h"
#include "interrupt.h"
#include "timer.h"
#include "rtc.h"
#include "keyboard.h"
#include "console.h"
#include "ata.h"

WORD* buf[1000];

void KernelMain()
{
    //__magic();    // break into BOCHS
    
    //__enableSSE();  // only for demo; in the future will be called from __init.asm

    ClearScreen();
	//__magic();

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
