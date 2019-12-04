#include "ata.h"
#include "stdfunc.h"
#include "timer.h"

WORD ata_buffer[256];

void detect_ata()
{
	int drive = 0;
	__outbyte(0x1F0 | ATA_REG_LBA0, 0x88);
	mSleep(10000);
	drive = __inbyte(0x1F0 | ATA_REG_LBA0);
	if (drive == 0x88)
	{
		__outbyte(0x1F0 | ATA_REG_HDDEVSEL, ATA_MASTER);
		drive = __inbyte(0x1F0 | ATA_REG_COMMAND);
		printf("drive = %d\n", drive);
		printf("HDD DETECTED\n");
	}

	__outbyte(0x173, 0x88);
	drive = __inbyte(0x1F0 | ATA_REG_LBA0);

	if (drive == 0x88)
	{
		printf("CDROM DETECTED\n");
	}
}

void identify_ata(WORD DriveBase, BYTE Type)
{
	BYTE status;
	BYTE error;
	int i;

	__outbyte(DriveBase + ATA_REG_HDDEVSEL, Type);
	__outbyte(DriveBase + ATA_REG_SECCOUNT0, 0);
	__outbyte(DriveBase + ATA_REG_LBA0, 0);
	__outbyte(DriveBase + ATA_REG_LBA1, 0);
	__outbyte(DriveBase + ATA_REG_LBA2, 0);
	__outbyte(DriveBase + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	status = __inbyte(DriveBase + ATA_REG_STATUS);
	if (status == 0)
	{
		printf("Drive does not exist\n");
		return;
	}
	while ((status = __inbyte(DriveBase + ATA_REG_STATUS)) & ATA_SR_BSY)
	{
		mSleep(10);
	}
	if ((__inbyte(DriveBase + ATA_REG_LBA1) | __inbyte(DriveBase + ATA_REG_LBA2)))
	{
		printf("Drive is not ATA\n");
		return;
	}
	while (((status = __inbyte(DriveBase + ATA_REG_STATUS)) & (ATA_SR_DRQ | ATA_SR_ERR)) == 0)
	{
		mSleep(10);
	}
	if (status & ATA_SR_ERR)
	{
		printf("Error\n");
		error = __inbyte(DriveBase + ATA_REG_ERROR);
		if (error & ATA_ER_BBK)		{ printf("\tBad Block Detected\n"); }
		if (error & ATA_ER_UNC)		{ printf("\tUncorrectable data error\n"); }
		if (error & ATA_ER_MC)		{ printf("\tMedia changed\n"); }
		if (error & ATA_ER_IDNF)	{ printf("\tID not found\n"); }
		if (error & ATA_ER_MCR)		{ printf("\tMedia change request\n"); }
		if (error & ATA_ER_ABRT)	{ printf("\tAborted command\n"); }
		if (error & ATA_ER_TK0NF)	{ printf("\tTrack zero not found\n"); }
		if (error & ATA_ER_AMNF)	{ printf("\tAddress mark not found\n"); }
		return;
	}

	printf("Drive %x succesful identified\n", DriveBase);

	for (i = 0; i < 256; i++)
	{
		ata_buffer[i] = __inword(DriveBase + ATA_REG_DATA);
		/*printf("D + %d: %x\n", i, ata_buffer[i]);
		__magic();*/
	}
}