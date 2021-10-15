/*
 * self_flashing.cpp
 *
 *  Created on: Oct 11, 2021
 *      Author: vitya
 */

#include "platform.h"

#if defined(BOARD_VRV153)

#include "hwspi.h"
#include "spiflash.h"
#include "clockcnt.h"
#include "traces.h"

#define SELFFLASH_BUFSIZE  256

extern const bootblock_header_t application_header;

extern unsigned __app_image_end;

// helper object to write memory contents into the SPI Flash memory
// it compares the flash content with the memory so it tries to avoid unnecessary write and erase
// It can operate with smaller temporary buffers, which can be allocated on the stack
class TSpiFlashWriter
{
public:
  TSpiFlash *  spiflash;
  uint8_t *    bufptr;
  unsigned     buflen;
  unsigned     sectorsize = 4096;
  
	bool         erased = true;

  TSpiFlashWriter(TSpiFlash * aspiflash, uint8_t * abufptr, unsigned abuflen)
  {
    spiflash = aspiflash;
    bufptr = abufptr;
    buflen = abuflen;
  }

  bool FlashSectorDiffer(unsigned asectoraddr, unsigned sectorlen, uint8_t * memptr)  // address and length alignment of 4 bytes are required
  {
    erased = true;

  	bool      differ = false;
    unsigned  remaining = sectorlen;
    unsigned  faddr = asectoraddr;

    while (remaining > 0)
    {
      unsigned chunksize = remaining;
      if (chunksize > buflen)  chunksize = buflen;

      spiflash->StartReadMem(faddr, bufptr, chunksize);
      spiflash->WaitForComplete();

      // compare memory

      uint32_t * mdp32  = (uint32_t *)(memptr);
      uint32_t * fdp32  = (uint32_t *)&(bufptr[0]);
      uint32_t * endptr = (uint32_t *)&(bufptr[chunksize]);

      while (fdp32 < endptr)
      {
        if (*fdp32 != 0xFFFFFFFF)
        {
          erased = false;
        }

        if (*fdp32 != *mdp32)
        {
          differ = false; // do not break for complete the erased check!
        }

        ++fdp32;
        ++mdp32;
      }

      faddr += chunksize;
      memptr += chunksize;
      remaining -= chunksize;
    }
    
  	return differ;
  }

  bool WriteToFlash(unsigned aflashaddr, uint8_t * asrc, unsigned alen)  // the flash address must begin on sector boundary !
  {
    if (!spiflash->initialized)
    {
      return false;
    }

    unsigned faddr = aflashaddr;
    uint8_t * srcptr = asrc;
    unsigned remaining = alen;

    while (remaining > 0)
    {
      unsigned chunksize = remaining;
      if (chunksize > sectorsize)  chunksize = sectorsize;

      if (FlashSectorDiffer(faddr, chunksize, srcptr))
      {
        if (!erased)
        {
          spiflash->StartEraseMem(faddr, chunksize);
          spiflash->WaitForComplete();
        }
        spiflash->StartWriteMem(faddr, srcptr, chunksize);
        spiflash->WaitForComplete();
      }

      faddr += chunksize;
      srcptr += chunksize;
      remaining -= chunksize;
    }

    return true;
  }
  
};

// do self flashing using the flash writer
bool spi_self_flashing(TSpiFlash * spiflash, unsigned flashaddr)
{
  uint8_t   localbuf[256] __attribute__((aligned(8)));
	unsigned  len = unsigned(&__app_image_end) - unsigned(&application_header);

	// Using the flash writer to first compare the flash contents:
  TSpiFlashWriter  flashwriter(spiflash, localbuf, sizeof(localbuf));

  TRACE("Self-Flashing:\r\n");
  TRACE("  mem = %08X -> flash = %08X, len = %u\r\n  ", memaddr, flashaddr, len);

  len = ((len + 7) & 0xFFFFFFF8); // length must be also dividible with 8 !

	unsigned  t0, t1;
	t0 = CLOCKCNT;

	if (!flashwriter.WriteToFlash(flashaddr, (uint8_t *)&application_header, len))
	{
    TRACE(" ERROR!\r\n", mainflash.errorcode);
    return false;
	}

	t1 = CLOCKCNT;

	unsigned clocksperus = SystemCoreClock / 1000000;

	TRACE("\r\n  Finished in %u us\r\n", (t1 - t0) / clocksperus);

	return true;
}

#endif
