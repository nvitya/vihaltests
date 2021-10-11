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

bool spi_self_flashing(TSpiFlash * spiflash, unsigned flashaddr)
{
	if (!spiflash->initialized)
	{
		return false;
	}

	unsigned  len = unsigned(&__app_image_end) - unsigned(&application_header);
	uint8_t   flasherbuf[SELFFLASH_BUFSIZE] __attribute__((aligned(4))); // buffer to read flash data
	unsigned t0, t1;

#if 0
	len = ((len + 7) & 0xFFFFFFF8); // length must be also dividible with 8 !

	TRACE("  mem = %08X -> flash = %08X, len = %u\r\n  ", memaddr, flashaddr, len);

	t0 = CLOCKCNT;

	unsigned   remaining = len;
	unsigned   chunksize = len;
	unsigned   faddr = flashaddr;
	uint8_t *  maddr = (uint8_t *)&application_header;
	uint32_t * mdp32;
	uint32_t * fdp32;
	uint32_t * endptr;

	uint32_t progresssize = 4096;
	uint32_t curprogress = 0;
	char progresschar = '.';

	while (remaining > 0)
	{
		// read the flash first
		chunksize = remaining;
		if (chunksize > SELFFLASH_BUFSIZE)  chunksize = SELFFLASH_BUFSIZE;

		spiflash->StartReadMem(faddr, flasherbuf, chunksize);
		spiflash->WaitForComplete();

		if (spiflash->errorcode != 0)
		{
			TRACE(" ERROR = %i\r\n", mainflash.errorcode);
			return false;
		}

		// compare memory
		bool   erased = true;
		bool   match = true;
		mdp32  = (uint32_t *)(maddr);
		fdp32  = (uint32_t *)&(flasherbuf[0]);
		endptr = (uint32_t *)&(flasherbuf[chunksize]);

		while (fdp32 < endptr)
		{
			if (*fdp32 != 0xFFFFFFFF)  erased = false;
			if (*fdp32 != *mdp32)
			{
				match = false; // do not break for complete the erased check!
			}

			++fdp32;
			++mdp32;
		}

		if (!match)
		{
			// must be rewritten

			if (!erased)
			{
				spiflash->StartEraseMem(faddr, chunksize);
				spiflash->WaitForComplete();
			}

			spiflash->StartWriteMem(faddr, maddr, chunksize);
			spiflash->WaitForComplete();
		}

		faddr += chunksize;
		maddr += chunksize;
		remaining -= chunksize;

		// progress info
		if (!match)
		{
			progresschar = 'W';
		}

		curprogress += chunksize;
		if ((curprogress >= progresssize) || (remaining == 0))
		{
			// write progress dot
			TRACE("%c", progresschar);

			progresschar = '.';
			curprogress = 0;
		}
	}
	// Compare the memory first

	t1 = CLOCKCNT;

	unsigned clocksperus = SystemCoreClock / 1000000;

	TRACE("\r\n  Finished in %u us\r\n", (t1 - t0) / clocksperus);

#endif

	return true;
}

#endif
