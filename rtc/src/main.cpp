// file:     main.cpp (uart)
// brief:    VIHAL UART Test
// created:  2021-10-03
// authors:  nvitya

#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "traces.h"

#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "hwrtc.h"

#include "board_pins.h"

THwRtc gRtc;

volatile unsigned hbcounter = 0;

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

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

  hwlsclk_init(true);

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	board_pins_init();
	gRtc.init();
	THwRtc::time_t startTime, lastTime;
	startTime.msec = 768;
	startTime.sec = 13;
	startTime.min = 43;
	startTime.hour = 13;
	startTime.day = 13;
	startTime.month = 03;
	startTime.year = 22;
	gRtc.setTime(startTime, lastTime);
  THwRtc::time_t aktTime;
  gRtc.getTime(aktTime);

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("%sHello From VIHAL !\r\n", CC_BLU);
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);


	TRACE("%02hhu:%02hhu:%02hhu.%03hu %02hhu.%02hhu.%02hhu\r\n", aktTime.hour, aktTime.min, aktTime.sec, aktTime.msec, aktTime.day, aktTime.month, aktTime.year);

  gRtc.enableWakeupIRQ();
  gRtc.setWakeupTimer(100);

	mcu_enable_interrupts();

	// Infinite loop
	while (1)
	{

	}
}

extern "C" void IRQ_Handler_03()
{
  ++hbcounter;
  if (hbcounter == 20)
  {
    gRtc.setWakeupTimer(500);
  }

  THwRtc::time_t aktTime;
  gRtc.getTime(aktTime);
  switch(hbcounter%8)
  {
  case 0:    TRACE("%s", CC_NRM);   break;
  case 1:    TRACE("%s", CC_RED);   break;
  case 2:    TRACE("%s", CC_GRN);   break;
  case 3:    TRACE("%s", CC_YEL);   break;
  case 4:    TRACE("%s", CC_BLU);   break;
  case 5:    TRACE("%s", CC_MAG);   break;
  case 6:    TRACE("%s", CC_CYN);   break;
  case 7:    TRACE("%s", CC_WHT);   break;
  }
  TRACE("%02hhu:%02hhu:%02hhu.%03hu %02hhu.%02hhu.%02hhu\r\n", aktTime.hour, aktTime.min, aktTime.sec, aktTime.msec, aktTime.day, aktTime.month, aktTime.year);

  gRtc.clearWakeupIRQ();
}

// ----------------------------------------------------------------------------
