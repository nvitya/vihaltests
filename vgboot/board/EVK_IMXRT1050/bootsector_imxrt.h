/*
 * bootsector_imxrt.h
 *
 *  Created on: Feb 8, 2018
 *      Author: nagy
 */

#ifndef BOOTSECTOR_IMXRT_H_
#define BOOTSECTOR_IMXRT_H_

#include "platform.h"

/*************************************
 *  IVT Data
 *************************************/
typedef struct _imxrt_ivt_t
{
    /** @ref hdr with tag #HAB_TAG_IVT, length and HAB version fields
     *  (see @ref data)
     */
    uint32_t hdr;
    /** Absolute address of the first instruction to execute from the
     *  image
     */
    uint32_t entry;
    /** Reserved in this version of HAB: should be NULL. */
    uint32_t reserved1;
    /** Absolute address of the image DCD: may be NULL. */
    uint32_t dcd;
    /** Absolute address of the Boot Data: may be NULL, but not interpreted
     *  any further by HAB
     */
    uint32_t boot_data;
    /** Absolute address of the IVT.*/
    uint32_t self;
    /** Absolute address of the image CSF.*/
    uint32_t csf;
    /** Reserved in this version of HAB: should be zero. */
    uint32_t reserved2;
} __attribute__((__packed__)) imxrt_ivt_t;

typedef struct _boot_data_
{
  uint32_t start;           /* boot start location */
  uint32_t size;            /* size */
  uint32_t plugin;          /* plugin flag - 1 if downloaded application is plugin */
  uint32_t placeholder;		/* placehoder to make even 0x10 size */
//
} __attribute__((__packed__)) imxrt_boot_data_t;


//!@brief FlexSPI LUT Sequence structure
typedef struct _lut_sequence
{
    uint8_t seqNum; //!< Sequence Number, valid number: 1-16
    uint8_t seqId;  //!< Sequence Index, valid number: 0-15
    uint16_t reserved;
//
} __attribute__((__packed__)) flexspi_lut_seq_t;

typedef struct _flexspi_nor_config
{
	// Common FlexSPIConfig
	//---------------------

  uint32_t tag;               //!< [0x000-0x003] Tag, fixed value 0x42464346UL
  uint32_t version;           //!< [0x004-0x007] Version,[31:24] -'V', [23:16] - Major, [15:8] - Minor, [7:0] - bugfix
  uint32_t reserved0;         //!< [0x008-0x00b] Reserved for future use
  uint8_t readSampleClkSrc;   //!< [0x00c-0x00c] Read Sample Clock Source, valid value: 0/1/3
  uint8_t csHoldTime;       //!< [0x00d-0x00d] CS hold time, default value: 3
  uint8_t csSetupTime;      //!< [0x00e-0x00e] CS setup time, default value: 3
  uint8_t columnAddressWidth; //!< [0x00f-0x00f] Column Address with, for HyperBus protocol, it is fixed to 3, For
  //! Serial NAND, need to refer to datasheet
  uint8_t deviceModeCfgEnable; //!< [0x010-0x010] Device Mode Configure enable flag, 1 - Enable, 0 - Disable
  uint8_t deviceModeType; //!< [0x011-0x011] Specify the configuration command type:Quad Enable, DPI/QPI/OPI switch,
  //! Generic configuration, etc.
  uint16_t waitTimeCfgCommands; //!< [0x012-0x013] Wait time for all configuration commands, unit: 100us, Used for
  //! DPI/QPI/OPI switch or reset command
  flexspi_lut_seq_t deviceModeSeq; //!< [0x014-0x017] Device mode sequence info, [7:0] - LUT sequence id, [15:8] - LUt
  //! sequence number, [31:16] Reserved
  uint32_t deviceModeArg;    //!< [0x018-0x01b] Argument/Parameter for device configuration
  uint8_t configCmdEnable;   //!< [0x01c-0x01c] Configure command Enable Flag, 1 - Enable, 0 - Disable
  uint8_t configModeType[3]; //!< [0x01d-0x01f] Configure Mode Type, similar as deviceModeTpe
  flexspi_lut_seq_t
      configCmdSeqs[3]; //!< [0x020-0x02b] Sequence info for Device Configuration command, similar as deviceModeSeq
  uint32_t reserved1;   //!< [0x02c-0x02f] Reserved for future use
  uint32_t configCmdArgs[3];     //!< [0x030-0x03b] Arguments/Parameters for device Configuration commands
  uint32_t reserved2;            //!< [0x03c-0x03f] Reserved for future use
  uint32_t controllerMiscOption; //!< [0x040-0x043] Controller Misc Options, see Misc feature bit definitions for more
  //! details
  uint8_t deviceType;    //!< [0x044-0x044] Device Type:  See Flash Type Definition for more details
  uint8_t sflashPadType; //!< [0x045-0x045] Serial Flash Pad Type: 1 - Single, 2 - Dual, 4 - Quad, 8 - Octal
  uint8_t serialClkFreq; //!< [0x046-0x046] Serial Flash Frequencey, device specific definitions, See System Boot
  //! Chapter for more details
  uint8_t lutCustomSeqEnable; //!< [0x047-0x047] LUT customization Enable, it is required if the program/erase cannot
  //! be done using 1 LUT sequence, currently, only applicable to HyperFLASH
  uint32_t reserved3[2];           //!< [0x048-0x04f] Reserved for future use
  uint32_t sflashA1Size;           //!< [0x050-0x053] Size of Flash connected to A1
  uint32_t sflashA2Size;           //!< [0x054-0x057] Size of Flash connected to A2
  uint32_t sflashB1Size;           //!< [0x058-0x05b] Size of Flash connected to B1
  uint32_t sflashB2Size;           //!< [0x05c-0x05f] Size of Flash connected to B2
  uint32_t csPadSettingOverride;   //!< [0x060-0x063] CS pad setting override value
  uint32_t sclkPadSettingOverride; //!< [0x064-0x067] SCK pad setting override value
  uint32_t dataPadSettingOverride; //!< [0x068-0x06b] data pad setting override value
  uint32_t dqsPadSettingOverride;  //!< [0x06c-0x06f] DQS pad setting override value
  uint32_t timeoutInMs;            //!< [0x070-0x073] Timeout threshold for read status command
  uint32_t commandInterval;        //!< [0x074-0x077] CS deselect interval between two commands
  uint16_t dataValidTime[2]; //!< [0x078-0x07b] CLK edge to data valid time for PORT A and PORT B, in terms of 0.1ns
  uint16_t busyOffset;       //!< [0x07c-0x07d] Busy offset, valid value: 0-31
  uint16_t busyBitPolarity;  //!< [0x07e-0x07f] Busy flag polarity, 0 - busy flag is 1 when flash device is busy, 1 -
  //! busy flag is 0 when flash device is busy
  uint16_t lookupTable[128];           //!< [0x080-0x17f] Lookup table holds Flash command sequences
  flexspi_lut_seq_t lutCustomSeq[12]; //!< [0x180-0x1af] Customizable LUT Sequences
  uint32_t reserved4[4];              //!< [0x1b0-0x1bf] Reserved for future use

  // Memory Type Specific config
	//---------------------

	uint32_t pageSize;              //!< Page size of Serial NOR
	uint32_t sectorSize;            //!< Sector size of Serial NOR
	uint8_t ipcmdSerialClkFreq;     //!< Clock frequency for IP command
	uint8_t isUniformBlockSize;     //!< Sector/Block size is the same
	uint8_t reserved5[2];           //!< Reserved for future use
	uint8_t serialNorType;          //!< Serial NOR Flash type: 0/1/2/3
	uint8_t needExitNoCmdMode;      //!< Need to exit NoCmd mode before other IP command
	uint8_t halfClkForNonReadCmd;   //!< Half the Serial Clock for non-read command: true/false
	uint8_t needRestoreNoCmdMode;   //!< Need to Restore NoCmd mode after IP commmand execution
	uint32_t blockSize;             //!< Block size
	uint32_t reserved6[11];          //!< Reserved for future use
//
} __attribute__((__packed__)) flexspi_nor_config_t;

extern const flexspi_nor_config_t   qspi_flash_config;
extern const imxrt_ivt_t            app_boot_ivt;
extern const imxrt_boot_data_t      app_boot_data;

#endif /* BOOTSECTOR_IMXRT_H_ */
