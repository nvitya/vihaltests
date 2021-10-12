
#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "spiflash.h"

THwSpi    spi;
TSpiFlash spiflash;

void start_app(unsigned appentry)
{
  __asm("jr %0" : : "r" (appentry));
}

// the C libraries require "_start" so we keep it as the entry point
extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required
{
	// run the C/C++ initialization:
	cppinit();

	// provide info to the system about the clock speed:
	SystemCoreClock = MCU_FIXED_SPEED;

	clockcnt_init();

	// go on with the hardware initializations
  spi.speed = 8000000;
  spi.Init(1); // flash

  spiflash.spi = &spi;
  spiflash.has4kerase = true;
  spiflash.Init();

	mcu_enable_interrupts();

	bootblock_header_t  apphead;

	spiflash.StartReadMem(BOOTBLOCK_STADDR, &apphead, sizeof(apphead));
	spiflash.WaitForComplete();

	if ((apphead.signature == BOOTBLOCK_SIGNATURE) && ((apphead.compid >> 16) == 0x0153))
	{
	  // load the body to the destination
	  spiflash.StartReadMem(BOOTBLOCK_STADDR, (void *)apphead.addr_load, apphead.length + sizeof(apphead));
	  spiflash.WaitForComplete();

	  //TODO: perform checksum

	  start_app(apphead.addr_entry);
	}

	// Infinite loop
	while (1)
	{

	}
}

// ----------------------------------------------------------------------------
