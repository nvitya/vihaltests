// file:     main_c1.cpp (uart_mp)
// brief:    VIHAL Multi-Core UART Test for Secondary core
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

#include "board_pins.h"

#include "app_header.h"
#include "multicore_app.h"
#include "hwmulticore.h"
#include "vgboot_utils.h"

volatile unsigned hbcounter = 0;

volatile uint32_t g_core1_counter = 0;

extern const TAppHeader application_header;

void request_secondary_self_flashing()
{
  TAppHeader *  papph = (TAppHeader *)&application_header;

  // calculate checksums
  papph->length = ((3 + papph->length) & 0x0FFFFFFC);  // round up length
  papph->csum_body = vgboot_checksum(papph + 1, papph->length);
  // the cusomdata will will be changed later to SECONDARY_SELF_FLASH_FLAG
  // but in order to avoid false data read on the primary we keep this at zero
  papph->customdata = 0;
  papph->csum_head = vgboot_checksum(papph, sizeof(TAppHeader) - 4) - SECONDARY_SELF_FLASH_FLAG;

  // set the customdata only when everything was setup perfectly
  papph->customdata = SECONDARY_SELF_FLASH_FLAG;  // this will be checked by the Primary core
}

volatile uint32_t g_ipc_irq_cnt = 0;
uint32_t prev_ipc_irq_cnt = 0;

extern "C" void IRQ_Handler_16()
{
  ++g_ipc_irq_cnt;

  uint32_t data;
  if (multicore.TryIpcRecv(&data))
  {
    //TRACE("C1 IPC IRQ data: %08X\r\n", data);
  }
  multicore.IpcIrqAck();
}

#define IRQPRIO_IPCINT  1

void enable_ipc_irq()
{
  int irqnum = SIO_PROC1_IRQn;

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

  // do not initialize the clock !

  SystemCoreClock = MCU_CLOCK_SPEED;

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();  // check if already initialized !!!
	uscounter.Init();
	multicore.Init(1);

	// go on with the hardware initializations
	board_pins_init();

	TRACE("\r\n----------------------------------------\r\n");
	TRACE("VIHAL UART Multi-Core Test on SECONDARY CORE\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  if (self_flashing)
  {
    request_secondary_self_flashing();
  }

	mcu_interrupts_enable();

	enable_ipc_irq();

	unsigned hbclocks = SystemCoreClock / 20;  // start blinking fast

	unsigned t0, t1;

  unsigned last_ipc_send_clocks = CLOCKCNT;
  unsigned ipc_send_interval = SystemCoreClock / 1000000;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		++g_core1_counter;

    if (t1 - last_ipc_send_clocks > ipc_send_interval)
    {
      multicore.TryIpcSend((uint32_t *)&g_core1_counter);
      last_ipc_send_clocks = t1;
    }

#if 0
		char c;
		if (conuart.TryRecvChar(&c))
		{
		  conuart.printf("you pressed \"%c\"\r\n", c);
		}
#endif

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
