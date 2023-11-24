// sdram_test.cpp

#include "platform.h"
#include "traces.h"
#include "clockcnt.h"
#include "hwsdram.h"


/** SDRAM benchmark test size */
#define SDRAMC_TEST_BUFF_SIZE        (30 * 1024)

/** SDRAM benchmark test page number */
#define SDRAMC_TEST_PAGE_NUM        (128) //(1024 * 1024 * SDRAM_MB_SIZE / SDRAMC_TEST_BUFF_SIZE - 2)

/** SDRAMC access test length */
#define SDRAMC_TEST_LENGTH        (10 * 1024)

/** SDRAMC access test data in even address */
#define SDRAMC_TEST_EVEN_TAG        (0x55aaaa55)

/** SDRAMC access test data in odd address */
#define SDRAMC_TEST_ODD_TAG        (0xaa5555aa)

void sdram_test()
{
	uint32_t i;
	uint32_t *pul = (uint32_t *)hwsdram.address;

	for (i = 0; i < SDRAMC_TEST_LENGTH; ++i)
	{
		if (i & 1) {
			pul[i] = SDRAMC_TEST_ODD_TAG | (1 << i);
		} else {
			pul[i] = SDRAMC_TEST_EVEN_TAG | (1 << i);
		}
	}

	bool bok = true;

	for (i = 0; i < SDRAMC_TEST_LENGTH; ++i)
	{
		if (i & 1)
		{
			if (pul[i] != (SDRAMC_TEST_ODD_TAG | (1u << i)))
			{
				TRACE("odd fail data %i: %08X\r\n", i, pul[i]);
				bok = false;
				break;
			}
		}
		else
		{
			if (pul[i] != (SDRAMC_TEST_EVEN_TAG | (1u << i)))
			{
				TRACE("even fail data %i: %08X\r\n", i, pul[i]);
				bok = false;
				break;
			}
		}
	}

	if (bok)
	{
		TRACE("SDRAM test successful.\r\n");
	}
	else
	{
		TRACE("SDRAM TEST FAILED !\r\n");
	}
}

void show_mem(void * aaddr, uint32_t alen)
{
	TRACE("Memory dump @ %08X, len = %u\r\n", aaddr, alen);
	uint8_t * dp = (uint8_t *)aaddr;
	for (uint32_t cnt = 0; cnt < alen; ++cnt)
	{
		TRACE(" %02X", *dp++);
		if ((cnt & 15) == 15)  TRACE("\r\n");
	}
	TRACE("\r\n");
}

uint32_t sdram_test_data = 0;

uint32_t g_checksum;

void show_mpu()
{
	TRACE("MPU settings:\r\n");
	int rcount = (MPU->TYPE >> 8);
	TRACE("region count = %i\r\n", rcount);
	for (int i = 0; i < rcount; ++i)
	{
		MPU->RNR = i;
		__DSB();
		TRACE("%i.: RBAR = %08X, RASR = %08X\r\n", i, MPU->RBAR, MPU->RASR);
	}
}

void mpu_setup()
{
	TRACE("Enabling Caching for the SDRAM\r\n");

	//show_mpu();
	MPU->CTRL = 0; // disable the MPU

	MPU->RNR = 0;
	MPU->RBAR = hwsdram.address;

	uint32_t attr = 0
		| (1 << 0) // shareable
	  | (1 << 1) // inner cache policy(2): 0 = non-cacheable, 1 = Write back, write and Read- Allocate, 2 = Write through, no Write-Allocate, 3 = Write back, no Write-Allocate
	  | (1 << 3) // outer cache policy(2): 0 = non-cacheable, 1 = Write back, write and Read- Allocate, 2 = Write through, no Write-Allocate, 3 = Write back, no Write-Allocate
 	  | (1 << 5) // cached region
  ;

	MPU->RASR = 0
		| (1  <<  0) // ENABLE
		| (27	<<  1) // SIZE(5): region size = 2^(1 + SIZE),  2^28 = 256 MByte
		| (0  <<  8) // SRD(8): subregion disable bits
		| (attr << 16) // attributes(6), B, S, C, TEX
		| (3  << 24) // AP(3): permissions, 3 = RW/RW (full access)
		| (0  << 28) // XN: 1 = code execution disabled
	;

	MPU->CTRL = (1 << 0) | (1 << 2); // enable MPU
}

void display_bm_res(uint32_t aclocks, uint32_t abytesize, uint32_t trsize)
{
	float ul_ticks = (float)aclocks / (SystemCoreClock / 1000);
	float ul_rw_speed = (float)abytesize / ul_ticks;
	float cpt = (float)(aclocks) / (float)(abytesize / trsize);
	TRACE("  Speed: %8.0f K/s, Clocks/transfer: %f\n\r", ul_rw_speed, cpt);
}

void sdram_test1()
{
	TRACE("SDRAM test1\r\n");

	TRACE("SDRAM address = %08X\r\n", hwsdram.address);

#if defined(MCUF_STM32) && (defined(MCUSF_F7) || defined(MCUSF_H7))
	//show_mpu();
	mpu_setup();
  //show_mpu();
#endif

	uint32_t i;
	uint16_t * startaddr = (uint16_t *)(hwsdram.address);

	uint32_t * dp = (uint32_t *) startaddr;
	uint32_t data = sdram_test_data;
	uint32_t len = hwsdram.byte_size / 4;

	TRACE("Filling whole memory...\r\n");

	for (i = 0; i < len; ++i)
	{
		//data = ~data;
		*dp++ = data + i;
	}

	TRACE("Reading back...\r\n");

	dp = (uint32_t *) startaddr;

	uint32_t errcnt = 0;

	for (i = 0; i < len; ++i)
	{
		//data = ~data;
		if (*dp++ != data + i)  ++errcnt;
	}

	TRACE("Error count = %u\r\n", errcnt);

	sdram_test_data += hwsdram.byte_size;


	unsigned t0, t1;
	uint32_t csum;
	uint32_t * endp;
	unsigned ul_ticks;
	unsigned ul_rw_speed;
	float    clocks_per_tr;
	float    mb_per_sec;

	TRACE("Benchmarking read speed...\r\n");

	endp = (uint32_t *)startaddr;
	endp += hwsdram.byte_size / 4;

	csum = 0;
	dp = (uint32_t *) startaddr;

	t0 = CLOCKCNT;

	while (dp < endp)
	{
		csum += *dp++;
	}

	t1 = CLOCKCNT;

	g_checksum += csum; // so that the optimizer keeps the code

	display_bm_res(t1 - t0, hwsdram.byte_size, 4);

	TRACE("Benchmarking read speed with 64 bit...\r\n");

	uint64_t * dp64 = (uint64_t *) startaddr;
	uint64_t * endp64 = dp64;
	endp64 += hwsdram.byte_size / 8;

	csum = 0;

	t0 = CLOCKCNT;

	while (dp64 < endp64)
	{
		csum += *dp64++;
	}

	t1 = CLOCKCNT;

	g_checksum += csum; // so that the optimizer keeps the code

	display_bm_res(t1 - t0, hwsdram.byte_size, 8);

	TRACE("Benchmarking WRITE32 speed...\r\n");

	endp = (uint32_t *)startaddr;
	endp += hwsdram.byte_size / 4;

	csum = 0;
	dp = (uint32_t *) startaddr;

	t0 = CLOCKCNT;

	while (dp < endp)
	{
		*dp++ = csum;
	}

	t1 = CLOCKCNT;

	display_bm_res(t1 - t0, hwsdram.byte_size, 4);

	uint32_t tcm_address = 0x20000000;
	uint32_t tcm_len = 64 * 1024;

	TRACE("Measuring TCM reference (32 bit tr.)...\r\n");

	len = tcm_len;

	dp = (uint32_t *)tcm_address;
	endp = dp;
	endp += len / 4;

	csum = 0;

	t0 = CLOCKCNT;

	while (dp < endp)
	{
		csum += *dp++;
	}

	t1 = CLOCKCNT;

	g_checksum += csum; // so that the optimizer keeps the code

	display_bm_res(t1 - t0, len, 4);

#if 0
	TRACE("Delaying 2 s...\r\n");

	delay_us(2000000);

	TRACE("Reading back...\r\n");

	dp = (uint32_t *) startaddr;

	errcnt = 0;

	for (i = 0; i < len; ++i)
	{
		if (*dp++ != data + i)  ++errcnt;
	}

	TRACE("Error count = %u\r\n", errcnt);
#endif

	//show_mem(startaddr, len * 4);

}


void sdram_fill()
{
	TRACE("Filling the SDRAM...\r\n");
	unsigned t0, t1;
	unsigned n;
	unsigned * dptr = (unsigned *)hwsdram.address;
	unsigned * endptr = (unsigned *)(hwsdram.address + hwsdram.byte_size);
	unsigned data = 0x12345678;

	t0 = CLOCKCNT;
	while (dptr != endptr)
	{
		*dptr++ = data;
	}
	t1 = CLOCKCNT;

	TRACE("  clocks = %u\r\n", t1-t0);
}

void sdram_fullread()
{
	TRACE("Reading the full SDRAM...\r\n");
	unsigned t0, t1;
	unsigned n;
	unsigned * dptr = (unsigned *)hwsdram.address;
	unsigned * endptr = (unsigned *)(hwsdram.address + hwsdram.byte_size);
	unsigned data = 0;

	t0 = CLOCKCNT;
	while (dptr != endptr)
	{
		data += *dptr++;
	}
	t1 = CLOCKCNT;

	TRACE("  clocks = %u, sum = %u\r\n", t1-t0, data);
}


/** SDRAM benchmark buffer */
static uint16_t gs_pus_mem_buff[SDRAMC_TEST_BUFF_SIZE];

static uint16_t gs_pus_mem_buff2[SDRAMC_TEST_BUFF_SIZE];

#define SAFE_RAM_ADDRESS (0x20010000 + 0x20000)

extern "C" void (* __isr_vectors [])();

/**
 * \brief SDRAM benchmark test.
 *
 * \return SDRAMC_OK for success and SDRAMC_ERROR for error.
 */
void sdram_benchmarks(void)
{
	uint16_t i;
	uint16_t *pus = (uint16_t *)hwsdram.address;
	uint32_t ul_tick_start, ul_tick_end, ul_rw_speed;
	uint32_t ul_ticks = 0;
	volatile uint16_t * dptr;
	volatile uint16_t * sptr;
	uint32_t ul_page_num = 0;

	TRACE("Start SDRAMC benchmarks.\r\n");

	for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
	{
		gs_pus_mem_buff[i] = (uint16_t)(i & 0xffff);
	}

	/* Start SDRAM write test */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = &gs_pus_mem_buff[0];
		dptr = &pus[ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//pus[i + ul_page_num *	SDRAMC_TEST_BUFF_SIZE] = gs_pus_mem_buff[i];
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SDRAMC write speed: %uK/s\n\r", (uint32_t)ul_rw_speed);


	/* Start SDRAM read test */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = &pus[ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		dptr = &gs_pus_mem_buff[0];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = pus[i + ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SDRAMC read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);


	// CHECK
	for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
	{
		if (gs_pus_mem_buff[i] != (i & 0xffff))
		{
			TRACE("-F- SDRAMC read data error\r\n");
			break; //return;
		}
	}

	uint16_t n;

	/* Start SDRAM read test */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = &pus[ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		dptr = &gs_pus_mem_buff[0];
		n = 0;
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = sptr[n];
			//sptr++;
			n = ((n + 111) & 0x3FFF);
			//gs_pus_mem_buff[i] = pus[i + ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SDRAMC nonseq read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);



	/* Start SDRAM to SDRAM test */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = &pus[ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		dptr = sptr + 32768;
		//dptr = &pus[(ul_page_num + 1) * SDRAMC_TEST_BUFF_SIZE];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = pus[i + ul_page_num * SDRAMC_TEST_BUFF_SIZE];
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SDRAM to SDRAM copy speed: %uK/s\n\r", (uint32_t)ul_rw_speed);

#if 0
	/* TCM reference */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = (uint16_t *)&gs_pus_mem_buff2[0];
		dptr = &gs_pus_mem_buff[0];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = *sptr++;
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("TCM reference read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);

	/* SRAM reference */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = (uint16_t *)SAFE_RAM_ADDRESS;
		dptr = &gs_pus_mem_buff[0];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = *sptr++;
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SRAM reference read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);
#endif

	/* FLASH reference */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = (uint16_t *)__isr_vectors;
		dptr = &gs_pus_mem_buff[0];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = *sptr++;
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("FLASH reference read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);

	/* FLASH nonseq reference */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		sptr = (uint16_t *)__isr_vectors;
		dptr = &gs_pus_mem_buff[0];
		n = 0;
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = sptr[n];
			//sptr++;
			n = ((n + 111) & 0x3FFF);
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("FLASH nonseq read speed: %uK/s\n\r", (uint32_t)ul_rw_speed);

#if 0
	/* SRAM reference */
	ul_tick_start = CLOCKCNT;
	for (ul_page_num = 0; ul_page_num < SDRAMC_TEST_PAGE_NUM; ul_page_num++)
	{
		dptr = (uint16_t *)SAFE_RAM_ADDRESS;
		sptr = &gs_pus_mem_buff[0];
		for (i = 0; i < SDRAMC_TEST_BUFF_SIZE; i++)
		{
			*dptr++ = *sptr++;
			//gs_pus_mem_buff[i] = *sptr++;
		}
	}
	ul_tick_end = CLOCKCNT;
	ul_ticks = (ul_tick_end - ul_tick_start) / (SystemCoreClock / 1000);
	ul_rw_speed = SDRAMC_TEST_BUFF_SIZE * SDRAMC_TEST_PAGE_NUM * sizeof(uint16_t) / ul_ticks;
	TRACE("SRAM reference write speed: %uK/s\n\r", (uint32_t)ul_rw_speed);
#endif


	TRACE("SDRAMC benchmark finished successfully.\r\n");
}

void sdram_tests()
{
	//tcm_speed();

	sdram_test1(); // full fill (with incrementing numbers) and read back

	//sdram_test();

	//delay_us(1000000);

	sdram_benchmarks();

	//sdram_fill();
	//sdram_fullread();
}




