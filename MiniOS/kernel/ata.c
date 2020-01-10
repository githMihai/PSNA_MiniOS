#include "ata.h"
#include "stdfunc.h"
#include "timer.h"

BYTE ata_buffer[256];

/* arguments:	First drive port base, first drive control base
				Second drive port base, second drive control base */
void ide_initialize(DWORD BAR0, DWORD BAR1, DWORD BAR2, DWORD BAR3, DWORD BAR4)
{
	int i, j, k, count = 0;
	BYTE err, type, status;

	// 1- Detect I/O Ports which interface IDE Controller:
	channels[ATA_PRIMARY].base = (BAR0 & 0xFFFFFFFC) + FIRST_DRIVE_PORT_BASE * (!BAR0);
	channels[ATA_PRIMARY].ctrl = (BAR1 & 0xFFFFFFFC) + FIRST_DRIVE_CONTROL_BASE * (!BAR1);
	channels[ATA_SECONDARY].base = (BAR2 & 0xFFFFFFFC) + SECOND_DRIVE_PORT_BASE * (!BAR2);
	channels[ATA_SECONDARY].ctrl = (BAR3 & 0xFFFFFFFC) + SECOND_DRIVE_CONTROL_BASE * (!BAR3);
	channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
	channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

	// 2- Disable IRQs:
	ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
	ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

	// 3- Detect ATA-ATAPI Devices:
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			err = 0;
			type = IDE_ATA;
			status;
			
			if ((type = ATAIdentify(i, ATA_MASTER | (j << 4))) == (IDE_UNKNOWN | IDE_NOT_FOUND))
			{
				continue;
			}
			// (VI) Read Device Parameters:
			ide_devices[count].Reserved = 1;
			ide_devices[count].Type = type;
			ide_devices[count].Channel = i;
			ide_devices[count].Drive = j;
			ide_devices[count].Signature = *((PWORD)(ata_buffer + ATA_IDENT_DEVICETYPE));
			ide_devices[count].Capabilities = *((PWORD)(ata_buffer + ATA_IDENT_CAPABILITIES));
			ide_devices[count].CommandSets = *((PWORD)(ata_buffer + ATA_IDENT_COMMANDSETS));

			// (VII) Get Size:
			if (ide_devices[count].CommandSets & (1 << 26))
			{
				// Device uses 48-Bit Addressing:
				ide_devices[count].LBA48BitAddressing = TRUE;
				ide_devices[count].LBA28BitAddressing = FALSE;
				ide_devices[count].Size = *((PDWORD)(ata_buffer + ATA_IDENT_MAX_LBA_EXT));
			}
			else
			{
				// Device uses CHS or 28-bit Addressing:
				ide_devices[count].LBA48BitAddressing = FALSE;
				ide_devices[count].LBA28BitAddressing = TRUE;
				ide_devices[count].Size = *((PDWORD)(ata_buffer + ATA_IDENT_MAX_LBA));
			}

			// (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
			for (k = 0; k < 40; k += 2) 
			{
				ide_devices[count].Model[k] = ata_buffer[ATA_IDENT_MODEL + k + 1];
				ide_devices[count].Model[k + 1] = ata_buffer[ATA_IDENT_MODEL + k];
			}
			ide_devices[count].Model[40] = 0; // Terminate String.

			ide_devices[count].IdentifyData = *(PIDENTIFY_DEVICE_DATA)((PWORD)(ata_buffer));

			count++;
		}
	}

	// 4- Print Summary:
	for (i = 0; i < 4; i++)
	{
		if (ide_devices[i].Reserved == 1)
		{
			printf(" Found %s Drive %dMB - %s - address %x\n",
				(const char* []) { "ATA", "ATAPI", "SATA" }[ide_devices[i].Type],         /* Type */
				//ide_devices[i].Size / 1024 / 1024 / 2,               /* Size */
				ide_devices[i].Size / 1024 / 2,               /* Size */
				ide_devices[i].Model,
				channels[ide_devices[i].Channel].base
			);
			if (ide_devices[i].LBA28BitAddressing)
			{
				printf(" 28 bit addressing mode\n");
			}
			else if (ide_devices[i].LBA48BitAddressing)
			{
				printf(" 48 bit addressing mode\n");
			}
			else
			{
				printf(" CHS mode\n");
			}
			printf("DMA: %d\n", ide_devices[i].IdentifyData.Capabilities.DmaSupported);
		}
	}
}

BYTE ide_read(BYTE channel, BYTE reg) {
	BYTE result;
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if (reg < 0x08)
		result = __inbyte(channels[channel].base + reg - 0x00);
	else if (reg < 0x0C)
		result = __inbyte(channels[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		result = __inbyte(channels[channel].ctrl + reg - 0x0A);
	else if (reg < 0x16)
		result = __inbyte(channels[channel].bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
	return result;
}

void ide_write(BYTE channel, BYTE reg, BYTE data) {
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
	if (reg < 0x08)
		__outbyte(channels[channel].base + reg - 0x00, data);
	else if (reg < 0x0C)
		__outbyte(channels[channel].base + reg - 0x06, data);
	else if (reg < 0x0E)
		__outbyte(channels[channel].ctrl + reg - 0x0A, data);
	else if (reg < 0x16)
		__outbyte(channels[channel].bmide + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

BYTE ATAGetError(BYTE channel)
{
	BYTE error;
	//error = __inbyte(DriveBase + ATA_REG_ERROR);
	error = ide_read(channel, ATA_REG_ERROR);
	if (error & ATA_ER_BBK) { printf("\tBad Block Detected\n"); }
	if (error & ATA_ER_UNC) { printf("\tUncorrectable data error\n"); }
	if (error & ATA_ER_MC) { printf("\tMedia changed\n"); }
	if (error & ATA_ER_IDNF) { printf("\tID not found\n"); }
	if (error & ATA_ER_MCR) { printf("\tMedia change request\n"); }
	if (error & ATA_ER_ABRT) { printf("\tAborted command\n"); }
	if (error & ATA_ER_TK0NF) { printf("\tTrack zero not found\n"); }
	if (error & ATA_ER_AMNF) { printf("\tAddress mark not found\n"); }
	return error;
}

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



//void ATAIdentify(WORD DriveBase, BYTE Type)
BOOLEAN ATAIdentify(BYTE channel, BYTE Type)
{
	BYTE status, error, cl, ch, type;
	DWORD data;
	int i;
	type = IDE_ATA;

	// (I) Select Drive:
	ide_write(channel, ATA_REG_HDDEVSEL, Type);

	// (II) Send Identify Command:
	ide_write(channel, ATA_REG_SECCOUNT0, 0);
	ide_write(channel, ATA_REG_LBA0, 0);
	ide_write(channel, ATA_REG_LBA1, 0);
	ide_write(channel, ATA_REG_LBA2, 0);
	ide_write(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	mSleep(10);

	// (III) Polling:
	status = ide_read(channel, ATA_REG_STATUS);
	if (status == 0)
	{
		printf("Drive does not exist\n");
		return IDE_NOT_FOUND;
	}
	while ((status = ide_read(channel, ATA_REG_STATUS)) & ATA_SR_BSY)
	{
		mSleep(10);
	}
	while (((status = ide_read(channel, ATA_REG_STATUS)) & (ATA_SR_DRQ | ATA_SR_ERR)) == 0)
	{
		if (status & ATA_SR_ERR)	
		{ 
			printf("Error\n");  
			ATAGetError(channel); 
			return IDE_NOT_FOUND;
		}
		mSleep(10);
	}

	// (IV) Probe for ATAPI Devices:
	cl = ide_read(channel, ATA_REG_LBA1);
	ch = ide_read(channel, ATA_REG_LBA2);

	if (cl == 0x0 && ch == 0x0)
		type = IDE_ATA;
	else if (cl == 0x14 && ch == 0xEB)
		type = IDE_ATAPI;
	else if (cl == 0x69 && ch == 0x96)
		type = IDE_ATAPI;
	else if (cl == 0x3c && ch == 0xc3)
		type = IDE_SATA;
	else
		return IDE_UNKNOWN;

	// (V)	Read Identification Space of the Device :
	for (i = 0; i < 128; i++)
	{
		data =  __indword(channels[channel].base + ATA_REG_DATA);
		*(((DWORD*)(ata_buffer))+i) = data;
	}

	return type;
}

BOOLEAN PoollingUntilReady(BYTE channel)
{
	BYTE status;
	int i;

	for (i = 0; i < 4; i++)
	{
		status = ide_read(channel, ATA_REG_ALTSTATUS);
	}

	while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
	{
		status = ide_read(channel, ATA_REG_STATUS);
		if (status & ATA_SR_ERR)
		{
			printf("Error\n");
			ATAGetError(channel);
			return FALSE;
		}
		if (status & ATA_SR_DF)
		{
			printf("Drive write fault\n");
			return FALSE;
		}
		if ((status & (ATA_SR_BSY | ATA_SR_DRQ)) == ATA_SR_DRQ)
		{
			return TRUE;
		}
	}
}

// TODO:
int ATAGetSectorCount(WORD DriveBase)
{
	BYTE sectorcount[2];
}

void PIO_read()
{

}

BYTE ide_ata_access(BYTE direction, BYTE drive, unsigned int lba, BYTE numsects, WORD* buffer)
{
	BYTE lba_mode, dma, cmd;
	BYTE lba_io[6];
	unsigned int channel = ide_devices[drive].Channel;
	unsigned int slavebit = ide_devices[drive].Drive;
	unsigned int bus = channels[channel].base;
	unsigned int words = 256;
	WORD cyl, i, j, k;
	BYTE head, sect, err;
	WORD dat;

	// (I) Select one from LBA28, LBA48 or CHS;
	if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are
							 // giving a wrong LBA.
	   // LBA48:
		lba_mode = 2;
		lba_io[0] = (lba & 0x000000FF) >> 0;
		lba_io[1] = (lba & 0x0000FF00) >> 8;
		lba_io[2] = (lba & 0x00FF0000) >> 16;
		lba_io[3] = (lba & 0xFF000000) >> 24;
		lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
		head = 0; // Lower 4-bits of HDDEVSEL are not used here.
	}
	else if (ide_devices[drive].Capabilities & 0x200) { // Drive supports LBA?
	// LBA28:
		lba_mode = 1;
		lba_io[0] = (lba & 0x00000FF) >> 0;
		lba_io[1] = (lba & 0x000FF00) >> 8;
		lba_io[2] = (lba & 0x0FF0000) >> 16;
		lba_io[3] = 0; // These Registers are not used here.
		lba_io[4] = 0; // These Registers are not used here.
		lba_io[5] = 0; // These Registers are not used here.
		head = (lba & 0xF000000) >> 24;
	}
	else {
		// CHS:
		lba_mode = 0;
		sect = (lba % 63) + 1;
		cyl = (lba + 1 - sect) / (16 * 63);
		lba_io[0] = sect;
		lba_io[1] = (cyl >> 0) & 0xFF;
		lba_io[2] = (cyl >> 8) & 0xFF;
		lba_io[3] = 0;
		lba_io[4] = 0;
		lba_io[5] = 0;
		head = (lba + 1 - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
	}

	// (II) See if drive supports DMA or not;
	dma = 0; // We don't support DMA

	// (III) Wait if the drive is busy;
	PoollingUntilReady(channel);

	// (IV) Select Drive from the controller;
	if (lba_mode == 0)
		ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head); // Drive & CHS.
	else
		ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA

	// (V) Write Parameters;
	if (lba_mode == 2) {
		ide_write(channel, ATA_REG_SECCOUNT1, 0);
		ide_write(channel, ATA_REG_LBA3, lba_io[3]);
		ide_write(channel, ATA_REG_LBA4, lba_io[4]);
		ide_write(channel, ATA_REG_LBA5, lba_io[5]);
	}
	ide_write(channel, ATA_REG_SECCOUNT0, numsects);
	ide_write(channel, ATA_REG_LBA0, lba_io[0]);
	ide_write(channel, ATA_REG_LBA1, lba_io[1]);
	ide_write(channel, ATA_REG_LBA2, lba_io[2]);

	if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
	if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
	if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;
	if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
	if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
	if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
	if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
	if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
	if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
	if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
	if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
	if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
	ide_write(channel, ATA_REG_COMMAND, cmd);               // Send the Command.

	if (dma)
		if (direction == 0);
	// DMA Read.
		else;
	// DMA Write.
	else
		if (direction == 0)
		{
			// PIO Read.
			j = 0;
			for (i = 0; i < numsects; i++) {
				if (!PoollingUntilReady(channel))
				{
					printf("Polling error\n");
					return err; // Polling, set error and exit if there is.
				}

				for (k = 0; k < SECTOR_SIZE/sizeof(WORD); k++)
				{
					dat = __inword(channels[channel].base + ATA_REG_DATA);
					buffer[j + k] = dat;
					nSleep(1);
				}
				j += SECTOR_SIZE / sizeof(WORD);
			}
		}
		else {
			// PIO Write.
			j = 0;
			for (i = 0; i < numsects; i++) {
				PoollingUntilReady(channel); // Polling.

				for (k = 0; k < SECTOR_SIZE / sizeof(WORD); k++)
				{
					__outword(channels[channel].base + ATA_REG_DATA, buffer[j + k]);
					nSleep(100);
				}
				j += SECTOR_SIZE / sizeof(WORD);
			}
			ide_write(channel, ATA_REG_COMMAND, (char[]) {
				ATA_CMD_CACHE_FLUSH,
					ATA_CMD_CACHE_FLUSH,
					ATA_CMD_CACHE_FLUSH_EXT
			}[lba_mode]);
			PoollingUntilReady(channel); // Polling.
		}

	return 0; // Easy, isn't it?
}

void ide_read_sectors(BYTE drive, BYTE numsects, unsigned int lba,
	WORD* buffer, unsigned int bufferSize) {
	// 1: Check if the drive presents:
	// ==================================
	if (drive > 3 || ide_devices[drive].Reserved == 0)
		printf("Drive Not found\n");      // Drive Not Found!

	// 2: Check if inputs are valid:
	// ==================================
	else if (((lba + numsects) > ide_devices[drive].Size) && (ide_devices[drive].Type == IDE_ATA))
		printf("Seeking to invalid position.\n");                     // Seeking to invalid position.

	// 3: Check buffer Size
	// ==================================
	else if (numsects * (SECTOR_SIZE / sizeof(WORD)) > bufferSize)
		printf("Buffer is too small\n");
	 // 4: Read in PIO Mode through Polling & IRQs:
	 // ============================================
	else {
		BYTE err;
		if (ide_devices[drive].Type == IDE_ATA)
			err = ide_ata_access(ATA_READ, drive, lba, numsects, buffer, bufferSize);
	}
}

void ide_write_sectors(BYTE drive, BYTE numsects, unsigned int lba,
	WORD* buffer, unsigned int bufferSize) {

	// 1: Check if the drive presents:
	// ==================================
	if (drive > 3 || ide_devices[drive].Reserved == 0)
		printf("Drive Not found\n");      // Drive Not Found!
	 // 2: Check if inputs are valid:
	 // ==================================
	else if (((lba + numsects) > ide_devices[drive].Size) && (ide_devices[drive].Type == IDE_ATA))
		printf("Seeking to invalid position.\n");                     // Seeking to invalid position.
	// 3: Check buffer Size
	// ==================================
	else if (numsects * (SECTOR_SIZE / sizeof(WORD)) > bufferSize)
		printf("Buffer is too small\n");
	 // 4: Read in PIO Mode through Polling & IRQs:
	 // ============================================
	else {
		BYTE err;
		if (ide_devices[drive].Type == IDE_ATA)
			err = ide_ata_access(ATA_WRITE, drive, lba, numsects, buffer);
	}
}
