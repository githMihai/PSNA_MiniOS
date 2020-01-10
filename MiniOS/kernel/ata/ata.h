#ifndef _ATA_H_
#define _ATA_H_

#include "main.h"

enum ATA_TYPE
{
	UNKNOWN,
	HDD,
	CDROM
};

// Channels:
#define ATA_PRIMARY					0x00
#define ATA_SECONDARY				0x01

#define IDE_ATA						0x00
#define IDE_ATAPI					0x01
#define IDE_SATA					0x02
#define IDE_UNKNOWN					0xFF
#define IDE_NOT_FOUND				0xFF

#define ATA_MASTER					0xA0
#define ATA_SLAVE					0xB0

#define ATA_REG_DATA				0x00
#define ATA_REG_ERROR				0x01
#define ATA_REG_FEATURES			0x01
#define ATA_REG_SECCOUNT0			0x02
#define ATA_REG_LBA0				0x03
#define ATA_REG_LBA1				0x04
#define ATA_REG_LBA2				0x05
#define ATA_REG_HDDEVSEL			0x06
#define ATA_REG_COMMAND				0x07
#define ATA_REG_STATUS				0x07
#define ATA_REG_SECCOUNT1			0x08
#define ATA_REG_LBA3				0x09
#define ATA_REG_LBA4				0x0A
#define ATA_REG_LBA5				0x0B
#define ATA_REG_CONTROL				0x0C
#define ATA_REG_ALTSTATUS			0x0C
#define ATA_REG_DEVADDRESS			0x0D

#define ATA_SR_BSY					0x80    // Busy
#define ATA_SR_DRDY					0x40    // Drive ready
#define ATA_SR_DF					0x20    // Drive write fault
#define ATA_SR_DSC					0x10    // Drive seek complete
#define ATA_SR_DRQ					0x08    // Data request ready
#define ATA_SR_CORR					0x04    // Corrected data
#define ATA_SR_IDX					0x02    // Index
#define ATA_SR_ERR					0x01    // Error

#define FIRST_DRIVE_PORT_BASE		0x1F0
#define FIRST_DRIVE_CONTROL_BASE	0x3F6
#define SECOND_DRIVE_PORT_BASE		0x170
#define SECOND_DRIVE_CONTROL_BASE	0x376

// Commands
#define ATA_CMD_READ_PIO			0x20
#define ATA_CMD_READ_PIO_EXT		0x24
#define ATA_CMD_READ_DMA			0xC8
#define ATA_CMD_READ_DMA_EXT		0x25
#define ATA_CMD_WRITE_PIO			0x30
#define ATA_CMD_WRITE_PIO_EXT		0x34
#define ATA_CMD_WRITE_DMA			0xCA
#define ATA_CMD_WRITE_DMA_EXT		0x35
#define ATA_CMD_CACHE_FLUSH			0xE7
#define ATA_CMD_CACHE_FLUSH_EXT		0xEA
#define ATA_CMD_PACKET				0xA0
#define ATA_CMD_IDENTIFY_PACKET		0xA1
#define ATA_CMD_IDENTIFY			0xEC

// ERRORS
#define ATA_ER_BBK					0x80    // Bad block
#define ATA_ER_UNC					0x40    // Uncorrectable data
#define ATA_ER_MC					0x20    // Media changed
#define ATA_ER_IDNF					0x10    // ID mark not found
#define ATA_ER_MCR					0x08    // Media change request
#define ATA_ER_ABRT					0x04    // Command aborted
#define ATA_ER_TK0NF				0x02    // Track 0 not found
#define ATA_ER_AMNF					0x01    // No address mark

#define ATA_IDENT_DEVICETYPE		0
#define ATA_IDENT_CYLINDERS			2
#define ATA_IDENT_HEADS				6
#define ATA_IDENT_SECTORS			12
#define ATA_IDENT_SERIAL			20
#define ATA_IDENT_MODEL				54
#define ATA_IDENT_CAPABILITIES		98
#define ATA_IDENT_FIELDVALID		106
#define ATA_IDENT_MAX_LBA			120
#define ATA_IDENT_COMMANDSETS		164
#define ATA_IDENT_MAX_LBA_EXT		200

// Directions:
#define ATA_READ					0x00
#define ATA_WRITE					0x01

#define SECTOR_SIZE					512

typedef struct _IDENTIFY_DEVICE_DATA {
	struct {
		WORD Reserved1 : 1;
		WORD Retired3 : 1;
		WORD ResponseIncomplete : 1;
		WORD Retired2 : 3;
		WORD FixedDevice : 1;
		WORD RemovableMedia : 1;
		WORD Retired1 : 7;
		WORD DeviceType : 1;
	} GeneralConfiguration;									// 0
	WORD NumCylinders;										// 2
	WORD SpecificConfiguration;								// 4
	WORD NumHeads;											// 6
	WORD Retired1[2];										// 8
	WORD NumSectorsPerTrack;								// 12
	WORD VendorUnique1[3];									// 14
	BYTE  SerialNumber[20];									// 20
	WORD Retired2[2];										// 40
	WORD Obsolete1;											// 44
	BYTE  FirmwareRevision[8];								// 46
	BYTE  ModelNumber[40];									// 54
	BYTE  MaximumBlockTransfer;								// 94
	BYTE  VendorUnique2;									// 95
	struct {
		WORD FeatureSupported : 1;
		WORD Reserved : 15;
	} TrustedComputing;										// 96
	struct {
		BYTE  CurrentLongPhysicalSectorAlignment : 2;
		BYTE  ReservedByte49 : 6;
		BYTE  DmaSupported : 1;
		BYTE  LbaSupported : 1;
		BYTE  IordyDisable : 1;
		BYTE  IordySupported : 1;
		BYTE  Reserved1 : 1;
		BYTE  StandybyTimerSupport : 1;
		BYTE  Reserved2 : 2;
		WORD ReservedWord50;
	} Capabilities;											// 98
	WORD ObsoleteWords51[2];								// 100
	WORD TranslationFieldsValid : 3;						// 104
	WORD Reserved3 : 5;										// 106
	WORD FreeFallControlSensitivity : 8;					// 108
	WORD NumberOfCurrentCylinders;							// 110
	WORD NumberOfCurrentHeads;								// 112
	WORD CurrentSectorsPerTrack;							// 114
	DWORD  CurrentSectorCapacity;							// 116
	BYTE  CurrentMultiSectorSetting;						// 120
	BYTE  MultiSectorSettingValid : 1;						// 	
	BYTE  ReservedByte59 : 3;								// 
	BYTE  SanitizeFeatureSupported : 1;						// 
	BYTE  CryptoScrambleExtCommandSupported : 1;			// 
	BYTE  OverwriteExtCommandSupported : 1;					// 
	BYTE  BlockEraseExtCommandSupported : 1;				// 
	DWORD  UserAddressableSectors;							// 124
	WORD ObsoleteWord62;									// 130
	WORD MultiWordDMASupport : 8;							// 
	WORD MultiWordDMAActive : 8;							// 
	WORD AdvancedPIOModes : 8;								// 
	WORD ReservedByte64 : 8;								// 
	WORD MinimumMWXferCycleTime;							// 134
	WORD RecommendedMWXferCycleTime;						// 136
	WORD MinimumPIOCycleTime;								// 138
	WORD MinimumPIOCycleTimeIORDY;							// 140
	struct {
		WORD ZonedCapabilities : 2;
		WORD NonVolatileWriteCache : 1;
		WORD ExtendedUserAddressableSectorsSupported : 1;
		WORD DeviceEncryptsAllUserData : 1;
		WORD ReadZeroAfterTrimSupported : 1;
		WORD Optional28BitCommandsSupported : 1;
		WORD IEEE1667 : 1;
		WORD DownloadMicrocodeDmaSupported : 1;
		WORD SetMaxSetPasswordUnlockDmaSupported : 1;
		WORD WriteBufferDmaSupported : 1;
		WORD ReadBufferDmaSupported : 1;
		WORD DeviceConfigIdentifySetDmaSupported : 1;
		WORD LPSAERCSupported : 1;
		WORD DeterministicReadAfterTrimSupported : 1;
		WORD CFastSpecSupported : 1;
	} AdditionalSupported;									// 142
	WORD ReservedWords70[5];								// 144
	WORD QueueDepth : 5;									// 154
	WORD ReservedWord75 : 11;								// 154
	struct {
		WORD Reserved0 : 1;
		WORD SataGen1 : 1;
		WORD SataGen2 : 1;
		WORD SataGen3 : 1;
		WORD Reserved1 : 4;
		WORD NCQ : 1;
		WORD HIPM : 1;
		WORD PhyEvents : 1;
		WORD NcqUnload : 1;
		WORD NcqPriority : 1;
		WORD HostAutoPS : 1;
		WORD DeviceAutoPS : 1;
		WORD ReadLogDMA : 1;
		WORD Reserved2 : 1;
		WORD CurrentSpeed : 3;
		WORD NcqStreaming : 1;
		WORD NcqQueueMgmt : 1;
		WORD NcqReceiveSend : 1;
		WORD DEVSLPtoReducedPwrState : 1;
		WORD Reserved3 : 8;
	} SerialAtaCapabilities;								// 156
	struct {
		WORD Reserved0 : 1;
		WORD NonZeroOffsets : 1;
		WORD DmaSetupAutoActivate : 1;
		WORD DIPM : 1;
		WORD InOrderData : 1;
		WORD HardwareFeatureControl : 1;
		WORD SoftwareSettingsPreservation : 1;
		WORD NCQAutosense : 1;
		WORD DEVSLP : 1;
		WORD HybridInformation : 1;
		WORD Reserved1 : 6;
	} SerialAtaFeaturesSupported;							// 160
	struct {
		WORD Reserved0 : 1;
		WORD NonZeroOffsets : 1;
		WORD DmaSetupAutoActivate : 1;
		WORD DIPM : 1;
		WORD InOrderData : 1;
		WORD HardwareFeatureControl : 1;
		WORD SoftwareSettingsPreservation : 1;
		WORD DeviceAutoPS : 1;
		WORD DEVSLP : 1;
		WORD HybridInformation : 1;
		WORD Reserved1 : 6;
	} SerialAtaFeaturesEnabled;								// 162
	WORD MajorRevision;										// 164
	WORD MinorRevision;										// 166
	struct {
		WORD SmartCommands : 1;
		WORD SecurityMode : 1;
		WORD RemovableMediaFeature : 1;
		WORD PowerManagement : 1;
		WORD Reserved1 : 1;
		WORD WriteCache : 1;
		WORD LookAhead : 1;
		WORD ReleaseInterrupt : 1;
		WORD ServiceInterrupt : 1;
		WORD DeviceReset : 1;
		WORD HostProtectedArea : 1;
		WORD Obsolete1 : 1;
		WORD WriteBuffer : 1;
		WORD ReadBuffer : 1;
		WORD Nop : 1;
		WORD Obsolete2 : 1;
		WORD DownloadMicrocode : 1;
		WORD DmaQueued : 1;
		WORD Cfa : 1;
		WORD AdvancedPm : 1;
		WORD Msn : 1;
		WORD PowerUpInStandby : 1;
		WORD ManualPowerUp : 1;
		WORD Reserved2 : 1;
		WORD SetMax : 1;
		WORD Acoustics : 1;
		WORD BigLba : 1;
		WORD DeviceConfigOverlay : 1;
		WORD FlushCache : 1;
		WORD FlushCacheExt : 1;
		WORD WordValid83 : 2;
		WORD SmartErrorLog : 1;
		WORD SmartSelfTest : 1;
		WORD MediaSerialNumber : 1;
		WORD MediaCardPassThrough : 1;
		WORD StreamingFeature : 1;
		WORD GpLogging : 1;
		WORD WriteFua : 1;
		WORD WriteQueuedFua : 1;
		WORD WWN64Bit : 1;
		WORD URGReadStream : 1;
		WORD URGWriteStream : 1;
		WORD ReservedForTechReport : 2;
		WORD IdleWithUnloadFeature : 1;
		WORD WordValid : 2;
	} CommandSetSupport;								// 168
	struct {
		WORD SmartCommands : 1;
		WORD SecurityMode : 1;
		WORD RemovableMediaFeature : 1;
		WORD PowerManagement : 1;
		WORD Reserved1 : 1;
		WORD WriteCache : 1;
		WORD LookAhead : 1;
		WORD ReleaseInterrupt : 1;
		WORD ServiceInterrupt : 1;
		WORD DeviceReset : 1;
		WORD HostProtectedArea : 1;
		WORD Obsolete1 : 1;
		WORD WriteBuffer : 1;
		WORD ReadBuffer : 1;
		WORD Nop : 1;
		WORD Obsolete2 : 1;
		WORD DownloadMicrocode : 1;
		WORD DmaQueued : 1;
		WORD Cfa : 1;
		WORD AdvancedPm : 1;
		WORD Msn : 1;
		WORD PowerUpInStandby : 1;
		WORD ManualPowerUp : 1;
		WORD Reserved2 : 1;
		WORD SetMax : 1;
		WORD Acoustics : 1;
		WORD BigLba : 1;
		WORD DeviceConfigOverlay : 1;
		WORD FlushCache : 1;
		WORD FlushCacheExt : 1;
		WORD Resrved3 : 1;
		WORD Words119_120Valid : 1;
		WORD SmartErrorLog : 1;
		WORD SmartSelfTest : 1;
		WORD MediaSerialNumber : 1;
		WORD MediaCardPassThrough : 1;
		WORD StreamingFeature : 1;
		WORD GpLogging : 1;
		WORD WriteFua : 1;
		WORD WriteQueuedFua : 1;
		WORD WWN64Bit : 1;
		WORD URGReadStream : 1;
		WORD URGWriteStream : 1;
		WORD ReservedForTechReport : 2;
		WORD IdleWithUnloadFeature : 1;
		WORD Reserved4 : 2;
	} CommandSetActive;									// 174
	WORD UltraDMASupport : 8;							// 180
	WORD UltraDMAActive : 8;							// 180
	struct {
		WORD TimeRequired : 15;
		WORD ExtendedTimeReported : 1;
	} NormalSecurityEraseUnit;							// 182
	struct {
		WORD TimeRequired : 15;
		WORD ExtendedTimeReported : 1;
	} EnhancedSecurityEraseUnit;						// 184
	WORD CurrentAPMLevel : 8;							// 186
	WORD ReservedWord91 : 8;							// 186
	WORD MasterPasswordID;								// 188
	WORD HardwareResetResult;							// 190
	WORD CurrentAcousticValue : 8;						// 192
	WORD RecommendedAcousticValue : 8;					// 192
	WORD StreamMinRequestSize;							// 194
	WORD StreamingTransferTimeDMA;						// 196
	WORD StreamingAccessLatencyDMAPIO;					// 198
	DWORD  StreamingPerfGranularity;					// 200
	DWORD  Max48BitLBA[2];								// 204
	WORD StreamingTransferTime;							// 212
	WORD DsmCap;										// 214
	struct {
		WORD LogicalSectorsPerPhysicalSector : 4;
		WORD Reserved0 : 8;
		WORD LogicalSectorLongerThan256Words : 1;
		WORD MultipleLogicalSectorsPerPhysicalSector : 1;
		WORD Reserved1 : 2;
	} PhysicalLogicalSectorSize;						// 216
	WORD InterSeekDelay;								// 218
	WORD WorldWideName[4];								// 220
	WORD ReservedForWorldWideName128[4];				// 228
	WORD ReservedForTlcTechnicalReport;					// 236
	WORD WordsPerLogicalSector[2];						// 238
	struct {
		WORD ReservedForDrqTechnicalReport : 1;
		WORD WriteReadVerify : 1;
		WORD WriteUncorrectableExt : 1;
		WORD ReadWriteLogDmaExt : 1;
		WORD DownloadMicrocodeMode3 : 1;
		WORD FreefallControl : 1;
		WORD SenseDataReporting : 1;
		WORD ExtendedPowerConditions : 1;
		WORD Reserved0 : 6;
		WORD WordValid : 2;
	} CommandSetSupportExt;								// 242
	struct {
		WORD ReservedForDrqTechnicalReport : 1;
		WORD WriteReadVerify : 1;
		WORD WriteUncorrectableExt : 1;
		WORD ReadWriteLogDmaExt : 1;
		WORD DownloadMicrocodeMode3 : 1;
		WORD FreefallControl : 1;
		WORD SenseDataReporting : 1;
		WORD ExtendedPowerConditions : 1;
		WORD Reserved0 : 6;
		WORD Reserved1 : 2;
	} CommandSetActiveExt;								// 244
	WORD ReservedForExpandedSupportandActive[6];		// 246
	WORD MsnSupport : 2;								// 258
	WORD ReservedWord127 : 14;							// 258
	struct {
		WORD SecuritySupported : 1;
		WORD SecurityEnabled : 1;
		WORD SecurityLocked : 1;
		WORD SecurityFrozen : 1;
		WORD SecurityCountExpired : 1;
		WORD EnhancedSecurityEraseSupported : 1;
		WORD Reserved0 : 2;
		WORD SecurityLevel : 1;
		WORD Reserved1 : 7;
	} SecurityStatus;									// 260
	WORD ReservedWord129[31];							// 262
	struct {
		WORD MaximumCurrentInMA : 12;
		WORD CfaPowerMode1Disabled : 1;
		WORD CfaPowerMode1Required : 1;
		WORD Reserved0 : 1;
		WORD Word160Supported : 1;
	} CfaPowerMode1;
	WORD ReservedForCfaWord161[7];
	WORD NominalFormFactor : 4;
	WORD ReservedWord168 : 12;
	struct {
		WORD SupportsTrim : 1;
		WORD Reserved0 : 15;
	} DataSetManagementFeature;
	WORD AdditionalProductID[4];
	WORD ReservedForCfaWord174[2];
	WORD CurrentMediaSerialNumber[30];
	struct {
		WORD Supported : 1;
		WORD Reserved0 : 1;
		WORD WriteSameSuported : 1;
		WORD ErrorRecoveryControlSupported : 1;
		WORD FeatureControlSuported : 1;
		WORD DataTablesSuported : 1;
		WORD Reserved1 : 6;
		WORD VendorSpecific : 4;
	} SCTCommandTransport;
	WORD ReservedWord207[2];
	struct {
		WORD AlignmentOfLogicalWithinPhysical : 14;
		WORD Word209Supported : 1;
		WORD Reserved0 : 1;
	} BlockAlignment;
	WORD WriteReadVerifySectorCountMode3Only[2];
	WORD WriteReadVerifySectorCountMode2Only[2];
	struct {
		WORD NVCachePowerModeEnabled : 1;
		WORD Reserved0 : 3;
		WORD NVCacheFeatureSetEnabled : 1;
		WORD Reserved1 : 3;
		WORD NVCachePowerModeVersion : 4;
		WORD NVCacheFeatureSetVersion : 4;
	} NVCacheCapabilities;
	WORD NVCacheSizeLSW;
	WORD NVCacheSizeMSW;
	WORD NominalMediaRotationRate;
	WORD ReservedWord218;
	struct {
		BYTE NVCacheEstimatedTimeToSpinUpInSeconds;
		BYTE Reserved;
	} NVCacheOptions;
	WORD WriteReadVerifySectorCountMode : 8;
	WORD ReservedWord220 : 8;
	WORD ReservedWord221;
	struct {
		WORD MajorVersion : 12;
		WORD TransportType : 4;
	} TransportMajorVersion;
	WORD TransportMinorVersion;
	WORD ReservedWord224[6];
	DWORD  ExtendedNumberOfUserAddressableSectors[2];
	WORD MinBlocksPerDownloadMicrocodeMode03;
	WORD MaxBlocksPerDownloadMicrocodeMode03;
	WORD ReservedWord236[19];
	WORD Signature : 8;
	WORD CheckSum : 8;
} IDENTIFY_DEVICE_DATA, * PIDENTIFY_DEVICE_DATA;

struct IDEChannelRegisters {
	unsigned short base;  // I/O Base.
	unsigned short ctrl;  // Control Base
	unsigned short bmide; // Bus Master IDE
	unsigned char  nIEN;  // nIEN (No Interrupt);
} channels[2];

struct ide_device {
	unsigned char	Reserved;			// 0 (Empty) or 1 (This Drive really exists).
	unsigned char	Channel;			// 0 (Primary Channel) or 1 (Secondary Channel).
	unsigned char	Drive;				// 0 (Master Drive) or 1 (Slave Drive).
	unsigned short	Type;				// 0: ATA, 1:ATAPI.
	unsigned short	Signature;			// Drive Signature
	unsigned short	Capabilities;		// Features.
	unsigned int	CommandSets;		// Command Sets Supported.
	unsigned int	Size;				// Size in Sectors.
	BOOLEAN			LBA28BitAddressing;	// 28 Bit Addressing
	BOOLEAN			LBA48BitAddressing;	// 48 Bit Addressing
	unsigned char	Model[41];			// Model in string.
	IDENTIFY_DEVICE_DATA IdentifyData;
	WORD			data[1000];
} ide_devices[4];

void ide_initialize(DWORD BAR0, DWORD BAR1, DWORD BAR2, DWORD BAR3, DWORD BAR4);
BYTE ide_read(BYTE channel, BYTE reg);
void ide_write(BYTE channel, BYTE reg, BYTE data);

//BYTE ATAGetError(WORD DriveBase);
BYTE ATAGetError(BYTE channel);

void detect_ata();

//void ATAIdentify(WORD DriveBase, BYTE Type);
BYTE ATAIdentify(BYTE channel, BYTE Type);

BOOLEAN PoollingUntilReady(BYTE channel);

int ATAGetSectorCount(WORD DriveBase);

void PIO_read();

BYTE ide_ata_access(BYTE direction, BYTE drive, unsigned int lba, BYTE numsect, WORD* buffer);

void ide_read_sectors(BYTE drive, BYTE numsects, unsigned int lba,
	WORD* buffer, unsigned int bufferSize);

void ide_write_sectors(BYTE drive, BYTE numsects, unsigned int lba,
	WORD* buffer, unsigned int bufferSize);

#endif