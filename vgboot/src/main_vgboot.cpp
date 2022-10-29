/* This file is a part of the VIHAL project: https://github.com/nvitya/vihal
 * Copyright (c) 2021 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 * file:     main_vgboot.cpp
 * brief:    VGBOOT Main Code
 * created:  2022-10-28
 * authors:  nvitya
 * notes:
 *   This is a generic bootloader for the devices that
 *     - does not have (enough) internal flash but have RAM where the code can run from.
 *     - does not have proper internal bootloader
 *
 *   In a real life project this bootloader is not enough, some form of application download
 *   should be supported.
*/

#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwclk.h"
#include "hwpins.h"
#include "traces.h"

#include "board_pins.h"

#include "app_header.h"
#include "vgboot_utils.h"

volatile unsigned hbcounter = 0;

void fatal_error()  // fast blinking all leds
{
  unsigned hbclocks = SystemCoreClock / 20;
  unsigned t0, t1;

  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    if (t1-t0 > hbclocks)
    {
      ++hbcounter;

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo(hbcounter & 1);
      }

      t0 = t1;
    }
  }
}

inline void start_app(unsigned appentry)
{
  __asm("bx %0" : : "r" (appentry));
}

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  TAppHeader    apphead; // this one is allocated on the stack!
  uint8_t *     pdest;
  uint32_t      csum;

  // after ram setup and region copy the cpu jumps here, with probably RC oscillator

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

  if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
  {
    while (1)
    {
      // error
    }
  }

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// basic hw init
	board_pins_init();

	// do only a few console output

	TRACE("\r\nVGBOOT START: %s\r\n", BOARD_NAME);

	board_init(); // second stage of the initialization

  if (!spiflash.Init())
  {
    TRACE("SPI Flash init failed!\r\n");
    fatal_error();
  }

  spiflash.StartReadMem(FLADDR_APPLICATION, &apphead, sizeof(apphead));
  spiflash.WaitForComplete();

  __DSB();

  if (apphead.signature != APP_HEADER_SIGNATURE)
  {
    TRACE("! APP HEADER SIGNATURE ERROR: %08X\r\n", apphead.signature);
    fatal_error();
  }

  csum = vgboot_checksum(&apphead, sizeof(apphead) - 4);  // skip the checksum from the calculation
  if (apphead.csum_head != csum)
  {
    TRACE("! APP HEADER CHECKSUM ERROR: %08X != %08X\r\n", apphead.csum_head, csum);
    fatal_error();
  }

  // load the whole to the destination (including the application header)
  //TRACE("APP LEN=%u\r\n", apphead.length);
  pdest = (uint8_t *)apphead.addr_load;
  spiflash.StartReadMem(FLADDR_APPLICATION, pdest, apphead.length + sizeof(apphead));
  spiflash.WaitForComplete();

  __DSB();

  // check the application checksum
  csum = vgboot_checksum(pdest + sizeof(apphead), apphead.length);
  if (apphead.csum_body != csum)
  {
    TRACE("! APP BODY CHECKSUM ERROR: %08X != %08X\r\n", apphead.csum_body, csum);
    fatal_error();
  }

  // everything is ok, start the app
  TRACE("%u bytes loaded to %08X, starting %08X ...\r\n",
      apphead.length + sizeof(apphead), apphead.addr_load, apphead.addr_entry);

  while (!conuart.SendFinished())
  {
    __NOP();
  }

  start_app(apphead.addr_entry);

  TRACE("! APP START ERROR: Jump failed\r\n");

  while (true)
  {
    fatal_error(); // this never returns
  }
}

// ----------------------------------------------------------------------------
