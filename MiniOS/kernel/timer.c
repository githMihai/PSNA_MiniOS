#include "timer.h"
#include "pit.h"
#include "screen.h"
#include "stdfunc.h"

static QWORD ticks;
static QWORD counter;

void TimerInit()
{
	ticks = 0;
	counter = 0;
	PitConfigureChannel(0, 2, TIMER_FREQ);
	// TODO: make a system to set the interrupt here
}


void TimerInterruptHandler()
{
	/*if (ticks % 1000000000 == 0)
	{
		ClearScreen();
		printf("ticks = %d\n", counter);
		counter++;
	}*/
	ticks++;
}

QWORD GetTicks()
{
	WORD old_int = GetIntFlags();
	__cli();
	QWORD t = ticks;
	SetInt(old_int);
	__sti();	// TODO: BUG
	return t;
}

QWORD ElapsedTicks(QWORD t)
{
	return GetTicks() - t;
}

void Sleep(QWORD ticks_loop)
{
	QWORD tick_start = GetTicks();
	QWORD t = ElapsedTicks(tick_start);
	QWORD t_n = ElapsedTicks(tick_start);
	while (t_n < ticks_loop)
	{
		if (t_n != t)
		{
			t = t_n;
		}
		t_n = ElapsedTicks(tick_start);
	}
}

void mSleep(QWORD milliseconds)
{
	Sleep((QWORD)((TIMER_FREQ / 1000.0) * milliseconds));
}

void uSleep(QWORD microseconds)
{
	Sleep((QWORD)((TIMER_FREQ / 1000000.0) * microseconds));
}

void nSleep(QWORD nanoseconds)
{
	Sleep((QWORD)((TIMER_FREQ / 1000000000.0) * nanoseconds));
}