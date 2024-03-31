/*-----------------------------------------------------------------------*/
/* Low level disk I/O implementations for ChaN FatFs                     */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "board_pins.h"
#include "clockcnt.h"
#include "traces.h"


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

extern "C" DSTATUS disk_status(BYTE pdrv)		// pdrv: Physical drive nmuber to identify the drive
{
	if (0 != pdrv)
	{
	  return STA_NOINIT;
	}

  if (!sdcard.card_initialized)
  {
    return STA_NODISK;
  }

  return 0;  // status ok.
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

extern "C" DSTATUS disk_initialize(BYTE pdrv)	// pdrv: Physical drive nmuber to identify the drive
{
  if (0 != pdrv)
  {
    return STA_NOINIT;
  }

  #ifdef SDCARD_SDMMC
    if (!sdcard.Init(&sd_mmc))
    {
      TRACE("SDCARD init error !\r\n");
      return STA_NOINIT;
    }
  #else
    if (!sdcard.Init(&sd_spi))
    {
      TRACE("SDCARD init error !\r\n");
      return STA_NOINIT;
    }
  #endif

  TRACE("Waiting for SDCARD initialization...\r\n");

  unsigned t0 = CLOCKCNT;
  while (true)
  {
    sdcard.Run();
    if (sdcard.card_initialized)
    {
      break;
    }
    if (CLOCKCNT - t0 > SystemCoreClock) // 1s timeout
    {
      return STA_NODISK;
    }
  }

  TRACE("SDCARD initialized, size = %u MB\r\n", sdcard.card_megabytes);

  return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

extern "C" DRESULT disk_read (
	BYTE   pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE * buff,		/* Data buffer to store read data */
	LBA_t  sector,	/* Start sector in LBA */
	UINT   count		/* Number of sectors to read */
)
{
  if (0 != pdrv)
  {
    return RES_PARERR;
  }

  sdcard.StartReadBlocks(sector,  buff, count);
  sdcard.WaitForComplete();
  if (sdcard.errorcode)
  {
    return RES_ERROR;
  }

  return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

extern "C" DRESULT disk_write (
	BYTE          pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *  buff,	    /* Data to be written */
	LBA_t         sector,		/* Start sector in LBA */
	UINT          count			/* Number of sectors to write */
)
{
  if (0 != pdrv)
  {
    return RES_PARERR;
  }

  sdcard.StartWriteBlocks(sector, (void *)buff, count);
  sdcard.WaitForComplete();
  if (sdcard.errorcode)
  {
    return RES_ERROR;
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

extern "C" DRESULT disk_ioctl (
	BYTE    pdrv,		/* Physical drive nmuber (0..) */
	BYTE    cmd,		/* Control code */
	void *  buff		/* Buffer to send/receive control data */
)
{
  if (0 != pdrv)
  {
    return RES_PARERR;
  }

  if (!sdcard.card_initialized)
  {
    return RES_NOTRDY;
  }

  DRESULT res = RES_OK;

  switch (cmd)
  {
    case CTRL_SYNC :    /* Make sure that no pending write process */
      break;

    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      *(DWORD*)buff = (DWORD) (sdcard.card_megabytes >> 9);
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = 512;
      break;

    case GET_BLOCK_SIZE : /* Get erase block size in unit of sector (DWORD) */
      *(DWORD*)buff = 512;
      break;

    default:
      res = RES_PARERR;
  }

	return res;
}

/*-----------------------------------------------------------------------*/
/* Time Stamping                                                         */
/*-----------------------------------------------------------------------*/

extern "C" DWORD get_fattime(void)
{
    return    ((DWORD)(2022 - 1980) << 25)    // Year 2022
            | ((DWORD)7  << 21)               // Month 7
            | ((DWORD)10 << 16)               // Mday 10
            | ((DWORD)16 << 11)               // Hour 16
            | ((DWORD)0  <<  5)               // Min 0
            | ((DWORD)0  >>  1)               // Sec 0
    ;
}
