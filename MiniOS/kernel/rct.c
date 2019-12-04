#include "rtc.h"
#include "interrupt.h"

void RTCInit()
{
	WORD intr_level;
	intr_level = GetIntLevel();
	__cli();
	__outbyte(0x70, 0x8B);		// select register B, and disable NMI
	char prev = __inbyte(0x71);	// read the current value of register B
	__outbyte(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
	__outbyte(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	SetInt(intr_level);
}

BYTE ReadCMOS(BYTE index)
{
	__outbyte(CMOS_REG_SET, index);
	return __inbyte(CMOS_REG_IO);
}

int BCDToDec(BYTE bcdval)
{
	/*int decval, temp;
	temp = (int)(bcdval / 16);
	decval = bcdval - 16 * temp;
	return decval;*/
	return (bcdval & 0x0f) + ((bcdval >> 4) * 10);
}

TIME SysGetTime()
{
	static const int days_per_month[12] =
	{
	  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	QWORD time;
	int sec, min, hour, day, month, year, i;

	sec = BCDToDec(ReadCMOS(RTC_REG_SEC));
	min = BCDToDec(ReadCMOS(RTC_REG_MIN));
	hour = BCDToDec(ReadCMOS(RTC_REG_HOUR));
	day = BCDToDec(ReadCMOS(RTC_REG_MDAY));
	month = BCDToDec(ReadCMOS(RTC_REG_MON));
	year = BCDToDec(ReadCMOS(RTC_REG_YEAR));

	time = (year * 365 + (year - 1) / 4) * 24 * 60 * 60;
	for (i = 1; i <= month; i++)
		time += days_per_month[i - 1] * 24 * 60 * 60;
	if (month > 2 && year % 4 == 0)
		time += 24 * 60 * 60;
	time += (day - 1) * 24 * 60 * 60;
	time += hour * 60 * 60;
	time += min * 60;
	time += sec;

	return time;
}

void SysGetDate(DATE* date)
{
	do
	{
		date->second = BCDToDec(ReadCMOS(RTC_REG_SEC));
		date->minute = BCDToDec(ReadCMOS(RTC_REG_MIN));
		date->hour = BCDToDec(ReadCMOS(RTC_REG_HOUR));
		date->day = BCDToDec(ReadCMOS(RTC_REG_MDAY));
		date->month = BCDToDec(ReadCMOS(RTC_REG_MON));
		date->year = BCDToDec(ReadCMOS(RTC_REG_YEAR));
	} while (date->second != BCDToDec(ReadCMOS(RTC_REG_SEC)));
}