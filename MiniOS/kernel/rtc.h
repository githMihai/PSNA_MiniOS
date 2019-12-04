#ifndef _RTC_H_
#define _RTC_H_

#include "main.h"

/* This code is an interface to the MC146818A-compatible real
   time clock found on PC motherboards.  See [MC146818A] for
   hardware details. */

   /* I/O register addresses. */
#define CMOS_REG_SET	0x70    /* Selects CMOS register exposed by REG_IO. */
#define CMOS_REG_IO		0x71    /* Contains the selected data byte. */

/* Indexes of CMOS registers with real-time clock functions.
   Note that all of these registers are in BCD format,
   so that 0x59 means 59, not 89. */
#define RTC_REG_SEC		0   /* Second: 0x00...0x59. */
#define RTC_REG_MIN		2   /* Minute: 0x00...0x59. */
#define RTC_REG_HOUR	4   /* Hour: 0x00...0x23. */
#define RTC_REG_MDAY	7	/* Day of the month: 0x01...0x31. */
#define RTC_REG_MON		8   /* Month: 0x01...0x12. */
#define RTC_REG_YEAR	9	/* Year: 0x00...0x99. */

/* Indexes of CMOS control registers. */
#define RTC_REG_A	0x0a    /* Register A: update-in-progress. */
#define RTC_REG_B	0x0b    /* Register B: 24/12 hour time, irq enables. */
#define RTC_REG_C	0x0c    /* Register C: pending interrupts. */
#define RTC_REG_D	0x0d    /* Register D: valid time? */

/* Register A. */
#define RTCSA_UIP	0x80	/* Set while time update in progress. */

/* Register B. */
#define	RTCSB_SET	0x80	/* Disables update to let time be set. */
#define RTCSB_DM	0x04	/* 0 = BCD time format, 1 = binary format. */
#define RTCSB_24HR	0x02    /* 0 = 12-hour format, 1 = 24-hour format. */

#pragma pack(push)
#pragma pack(1)
typedef struct _DATE
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
}DATE;
#pragma pack(pop)

typedef unsigned long TIME;

void RTCInit();

TIME SysGetTime();
void SysGetDate(DATE* date);
BYTE ReadCMOS(BYTE index);
int BCDToDec(BYTE bcdval);

#endif // _RTC_H_