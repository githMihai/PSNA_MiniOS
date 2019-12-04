#include "pit.h"
#include "main.h"
#include "interrupt.h"

void PitConfigureChannel(int channel, int mode, int frequency)
{
	__cli();
	WORD count;
	if (frequency > PIT_FREQ)
	{
		count = 2;
	}
	else
	{
		count = (PIT_FREQ + frequency / 2) / frequency;
	}

	WORD old_int = GetIntFlags();
	__cli();
	__outbyte(PIT_PORT_CONTROL, (channel << 6) | 0x30 | (mode << 1));
	__outbyte(PIT_PORT_COUNTER(channel), count);
	__outbyte(PIT_PORT_COUNTER(channel), count >> 8);
	SetInt(old_int);
}