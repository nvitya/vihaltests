// bootsector_1020.cpp

#include "bootsector_imxrt.h"

__attribute__((section(".flexspi_boot_cfg"),used))
const flexspi_nor_config_t qspi_flash_config =
{
	.tag = 0x42464346,
	.version = 0x56010400,
	.reserved0 = 0,
	.readSampleClkSrc = 0,
	.csHoldTime = 3,
	.csSetupTime = 3,
	.columnAddressWidth = 0,

	.deviceModeCfgEnable = 0,
	.deviceModeType = 0,
	.waitTimeCfgCommands = 0,
	.deviceModeSeq = {0, 0, 0},
	.deviceModeArg = 0,

	.configCmdEnable = 0,
	.configModeType = {0, 0, 0},
	.configCmdSeqs =	{
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
		},
	.reserved1 = 0,
	.configCmdArgs = {0, 0, 0},
	.reserved2 = 0,

	.controllerMiscOption = 0,

	.deviceType = 1,      // 1 = serial NOR
	.sflashPadType = 2,   // 2 = dual mode
	.serialClkFreq = 1,   // 1 = use 30 MHz clock (lowest)

	.lutCustomSeqEnable = 0,
	.reserved3 = {0, 0},

	.sflashA1Size = 1 * 1024 * 1024,  // 1 MByte by default
	.sflashA2Size = 0,
	.sflashB1Size = 0,
	.sflashB2Size = 0,

	.csPadSettingOverride = 0,
	.sclkPadSettingOverride = 0,
	.dataPadSettingOverride = 0,
	.dqsPadSettingOverride = 0,
	.timeoutInMs = 0,
	.commandInterval = 0,
	.dataValidTime = {0, 0}, //{16, 16},
	.busyOffset = 0,
	.busyBitPolarity = 0,
	.lookupTable = {

		#define LUTINS(acmd, apads, adata)  (adata | (apads << 8) | (acmd << 10))
		// apads: 0 = 1x line, 1 = 2x, 2 = 4x, 3 = 8x

		// LUT commands:
		//  0: end of LUT instuction sequence
		//  1: transmit command code (usually the first)
		//  2: transmit row address, data = bit number
		//  6: transmit 4 mode bits, data = mode bits
		//  7: transmit 8 mode bits, data = mode bits
		//  8: transmit programming data to flash
		//  9: receive data from flash
		// 12: dummy cycles, data = clock count (usually 8 required)

		//  0: DUAL I/O READ
			LUTINS(1,0,0xBB), LUTINS(2,1,24), LUTINS(6,1,0x00), LUTINS(12,1,2), LUTINS(9,1,0), 0, 0, 0,
		//  1-15: empty
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
			0, 0, 0, 0,	0, 0, 0, 0,
		},
	.lutCustomSeq = {
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
		},
	.reserved4 = {0, 0, 0, 0},
	.pageSize = 256,
	.sectorSize = 4 * 1024,

	.ipcmdSerialClkFreq = 0, // keep the current clock freq
	.isUniformBlockSize = true,
	.reserved5 = {0, 0},
	.serialNorType = 0,
	.needExitNoCmdMode = 0,
	.halfClkForNonReadCmd = 0,
	.needRestoreNoCmdMode = 0,
	.blockSize = 256 * 1024,
	.reserved6 = {0,0,0,0,0,0,0,0,0,0,0}
};

extern "C" void cold_entry(); // the application must define this

__attribute__((section(".flexspi_boot_ivt"),used))
const imxrt_ivt_t app_boot_ivt =
{
	.hdr = (
			0xD1            // fix tag
			| (0x2000 << 8) // ivt size
			| (0x41 << 24)  // ivt version
	),
	.entry = (unsigned)cold_entry,
  .reserved1 = 0,
	.dcd = 0,           // no device configuration present
	.boot_data = BOOTLOADER_LOAD_ADDR + 0x1020,
	.self      = BOOTLOADER_LOAD_ADDR + 0x1000, // loaded into this fixed address
	.csf = 0,           // required only for secure boot
	.reserved2 = 0
};


__attribute__((section(".flexspi_boot_data"),used))
const imxrt_boot_data_t app_boot_data =
{
	.start = BOOTLOADER_LOAD_ADDR,
	.size  = BOOTLOADER_MAX_SIZE,
	.plugin = 0,
	.placeholder = 0xFFFFFFFF
};
