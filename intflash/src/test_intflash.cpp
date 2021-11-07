/*
 *  file:     test_intflash.cpp
 *  brief:    Internal Flash write example for NVCM
 *  version:  1.00
 *  date:     2019-04-06
 *  authors:  nvitya
*/

#include "stdint.h"
#include "stdlib.h"
#include "platform.h"
#include "hwpins.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "traces.h"

#include "hwintflash.h"

#ifndef FLASH_ERASE_VALUE
  #define FLASH_ERASE_VALUE 0xFFFFFFFF
#endif

unsigned clocks_to_us(unsigned aclocks)
{
	unsigned clocks_per_us = SystemCoreClock / 1000000;
	return (aclocks + (clocks_per_us >> 1)) / clocks_per_us;
}

void test_intflash_addr(uint32_t testaddr, uint32_t testlen, uint8_t * pbuf)
{
	unsigned    t0, t1;
	uint32_t *  p1;
	uint32_t *  p2;
	int i;
	bool bok = true;

	TRACE("Test parameters:\r\n");
	TRACE("  Address: 0x%08X\r\n", testaddr);
	TRACE("  Length : %u k\r\n", testlen >> 10);

	uint32_t testdwcnt = (testlen >> 2);

	bok = true;

	if (bok)
	{
		// first erase the memory
		TRACE("Erasing test area...\r\n");
		t0 = CLOCKCNT;
		hwintflash.StartEraseMem(testaddr, testlen);
		hwintflash.WaitForComplete();
		t1 = CLOCKCNT;
		TRACE("  Erase time = %u us\r\n", clocks_to_us(t1 - t0));

		TRACE("Verifying...\r\n");

		p1 = (uint32_t *)(testaddr);
		for (i = 0; i < testdwcnt; ++i)
		{
			if (*p1 != FLASH_ERASE_VALUE)
			{
				TRACE("  Not erased at 0x%08X !\r\n", p1);
				bok = false;
				break;
			}
			++p1;
		}

		if (bok)
		{
			TRACE("  OK.\r\n");
		}
	}

	if (bok)
	{
		TRACE("Filling test area...\r\n");

		p1 = (uint32_t *)(pbuf);
		for (i = 0; i < testdwcnt; ++i)
		{
			*p1++ = 0x04030201 * i;
		}

		t0 = CLOCKCNT;
		hwintflash.StartWriteMem(testaddr, pbuf, testlen);
		hwintflash.WaitForComplete();
		t1 = CLOCKCNT;
		TRACE("  Write time = %u us\r\n", clocks_to_us(t1 - t0));

		TRACE("Verifying...\r\n");

		p1 = (uint32_t *)(pbuf);
		p2 = (uint32_t *)(testaddr);
		for (i = 0; i < testdwcnt; ++i)
		{
			if (*p1 != *p2)
			{
				bok = false;
				TRACE("  Mismatch at 0x%08X !\r\n", p2);
				break;
			}
			++p1;
			++p2;
		}

		if (bok)
		{
			TRACE("  OK.\r\n");
		}
	}


#if 1
	if (bok)
	{
		TRACE("Testing Copy...\r\n");

		// prepare another content
		p1 = (uint32_t *)(pbuf);
		for (i = 0; i < testdwcnt; ++i)
		{
			*p1++ = 0x01010101 * i;
		}

		t0 = CLOCKCNT;
		hwintflash.StartCopyMem(testaddr, pbuf, testlen);
		hwintflash.WaitForComplete();
		t1 = CLOCKCNT;
		TRACE("  Copy time = %u us\r\n", clocks_to_us(t1 - t0));

		TRACE("Verifying...\r\n");

		p1 = (uint32_t *)(pbuf);
		p2 = (uint32_t *)(testaddr);
		for (i = 0; i < testdwcnt; ++i)
		{
			if (*p1 != *p2)
			{
				bok = false;
				TRACE("  Mismatch at 0x%08X !\r\n", p2);
				break;
			}
			++p1;
			++p2;
		}

		if (bok)
		{
			TRACE("  OK.\r\n");
		}
	}
#endif


	TRACE("Internal Flash Write Test finished.\r\n");
}

void test_intflash()
{
  TRACE("Internal Flash Write test\r\n");

	if (!hwintflash.Init())
	{
		TRACE("Error initializing internal flash !\r\n");
		return;
	}

	hwintflash.TraceFlashInfo();

#if 1

	unsigned testlen = 0x10000;
	uint8_t * pmem = nullptr;
	while (!pmem)
	{
		pmem = (uint8_t *)malloc(testlen);
		if (pmem)  break;
		testlen = (testlen >> 1); // try smaller mem
	}

	for (unsigned runcnt = 0; runcnt < 2; ++runcnt)
	{
		if (runcnt > 0)
		{
		  TRACE("Reapeating Write Test\r\n");
		}

#if defined(MCUF_XMC)
		test_intflash_addr(0x0C010000, testlen, pmem);
#else
		test_intflash_addr(hwintflash.start_address + hwintflash.bytesize / 2, testlen, pmem);
#endif
	}

	if (pmem)
	{
		free(pmem);
	}

#endif

	TRACE("Internal Flash Write Test finished.\r\n");
}

// ----------------------------------------------------------------------------
