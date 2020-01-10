#ifndef _PIT_H_
#define _PIT_H_

// PROGRAMMABLE INTERVAL TIMER

#define PIT_PORT_CONTROL			0x43
#define PIT_PORT_COUNTER(CHANNEL)	(0x40 + (CHANNEL))
#define PIT_FREQ					1193180

void PitConfigureChannel(int channel, int mode, int frequency);

#endif // _PIT_H_