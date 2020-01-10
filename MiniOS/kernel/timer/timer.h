#ifndef _TIMER_H
#define _TIMER_H_

#include "main/main.h"

#define TIMER_FREQ	100

void TimerInit();
void TimerInterruptHandler();
QWORD GetTicks();
QWORD ElapsedTicks(QWORD t);

void Sleep(QWORD ticks_loop);
void mSleep(QWORD milliseconds);
void uSleep(QWORD microseconds);
void nSleep(QWORD nanoseconds);

#endif // _TIMER_H_