// file:     main_c0.cpp (uart_mp)
// brief:    VIHAL Multi-Core UART Test for Primary core
// created:  2024-06-15
// authors:  nvitya

#include "platform.h"
//#include "hwclkctrl.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwuscounter.h"
#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"
#include "hwmulticore.h"

#include "board_pins.h"

#include "secondary_self_flash.h"

#if SPI_SELF_FLASHING
  #include "spi_self_flashing.h"
#endif

volatile unsigned hbcounter = 0;

void check_us_counter()
{
  const int uscap_len = 64;
  uint32_t  usarr[uscap_len];


  TRACE("Testing us counter...\r\n");

  uint32_t  ccnt = 0;
  unsigned t0, t1;
  unsigned usclocks = SystemCoreClock / 1000000;

  t0 = CLOCKCNT;
  while (ccnt < uscap_len)
  {
    t1 = CLOCKCNT;
    if (t1 - t0 > usclocks)
    {
      usarr[ccnt]  = uscounter.Get32();
      ++ccnt;
      t0 += usclocks;
    }
  }

  // displaying
  for (ccnt = 0; ccnt < uscap_len; ++ccnt)
  {
    TRACE("%3i. = %09u\r\n", ccnt, usarr[ccnt]);
  }
}

void check_us_counter_readout_speed()
{
  uint32_t t0, t1, t2;

  t0 = CLOCKCNT;
  if (uscounter.Get32()) {}
  t1 = CLOCKCNT;
  if (uscounter.Get32()) {}
  t2 = CLOCKCNT;
  TRACE("uscounter read clocks: %u, %u\r\n", t1-t0, t2-t1);
}

uint32_t g_core0_counter = 0;
volatile uint32_t g_ipc_irq_cnt = 0;
uint32_t prev_ipc_irq_cnt = 0;

extern "C" void IRQ_Handler_15()
{
  ++g_ipc_irq_cnt;

  uint32_t data;
  if (multicore.TryIpcRecv(&data))
  {
    //TRACE("IPC IRQ data: %08X\r\n", data);
  }
  multicore.IpcIrqAck();
}

#define IRQPRIO_IPCINT  1

void enable_ipc_irq()
{
  int irqnum = SIO_PROC0_IRQn;

  mcu_irq_priority_set(irqnum, IRQPRIO_IPCINT);
  mcu_irq_pending_clear(irqnum);
  mcu_irq_enable(irqnum);
}

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
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
	uscounter.Init();
  multicore.Init(0);

	// go on with the hardware initializations
	board_pins_init();

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("VIHAL UART Multi-Core Test on PRIMARY CORE\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

#if SPI_SELF_FLASHING

  if (spiflash.initialized)
  {
    TRACE("SPI Flash ID CODE: %08X, size = %u\r\n", spiflash.idcode, spiflash.bytesize);
  }
  else
  {
    TRACE("Error initializing SPI Flash !\r\n");
  }

  if (self_flashing)
  {
    spi_self_flashing(&spiflash);
  }
#endif

  if (!check_secondary_self_flash())
  {
    // try to load the secondary core code
    load_secondary_core_code();
  }

  //check_us_counter();
  check_us_counter_readout_speed();

	mcu_interrupts_enable();

	enable_ipc_irq();

	TRACE("Starting Main Cycle...\r\n");

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast

	unsigned t0, t1;

	unsigned last_ipc_send_clocks = CLOCKCNT;
	unsigned ipc_send_interval = SystemCoreClock / 1000000;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		++g_core0_counter;

		if (t1 - last_ipc_send_clocks > ipc_send_interval)
		{
		  multicore.TryIpcSend((uint32_t *)&g_core0_counter);
		  last_ipc_send_clocks = t1;
		}

		check_secondary_self_flash();

		char c;
		if (conuart.TryRecvChar(&c))
		{
		  //conuart.printf("you pressed \"%c\"\r\n", c);
		}

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

			TRACE("hbcounter=%u, ipc_irq_cnt=%u\r\n", hbcounter, g_ipc_irq_cnt - prev_ipc_irq_cnt);
			prev_ipc_irq_cnt = g_ipc_irq_cnt;

			t0 = t1;

			if (hbcounter > 20)  hbclocks = SystemCoreClock / 2;  // slow down to 0.5 s
		}
	}
}

// ----------------------------------------------------------------------------
