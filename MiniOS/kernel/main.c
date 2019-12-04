#include "main.h"
#include "screen.h"
#include "interrupt.h"
#include "timer.h"
#include "rtc.h"
#include "keyboard.h"
#include "console.h"
#include "ata.h"

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
	identify_ata(FIRST_DRIVE_PORT_BASE, ATA_MASTER);
	//__magic();
	ExecuteConsole();

	int i = 0;
	while (1)
	{
		//printf("Mama are mere %d\n", i);
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
