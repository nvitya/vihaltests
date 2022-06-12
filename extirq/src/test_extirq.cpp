/*
 *  file:     test_extirq.cpp
 *  brief:    External IRQ Test
 *  date:     2021-11-13
 *  authors:  nvitya
 *
 *  description:
 *    The pins "pin_irqctrl" and "pin_extirq" must be connected externally !
 *    Toggling the "pin_irqctrl" thus causes an IRQ at rising edge, which normally happens every 2 s.
 *    The IRQ accept latency is measured in CPU clocks.
*/

#include "stdint.h"
#include "stdlib.h"
#include "platform.h"
#include "hwpins.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "board_pins.h"
#include "hwextirq.h"

#include "traces.h"

#define IRQPRIO_EXTINT   5

THwExtIrq          extirq;
volatile unsigned  g_extirq_start_time;
volatile unsigned  g_extirq_cnt = 0;
volatile unsigned  g_extirq_accept_time = 0;

unsigned           g_extirq_cnt_prev = 0;

void setup_irq(int airqnum)
{
  IRQn_Type irqnum = IRQn_Type(airqnum);
  NVIC_SetPriority(irqnum, IRQPRIO_EXTINT);
  NVIC_ClearPendingIRQ(irqnum);
  NVIC_EnableIRQ(irqnum);
}

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_MIN_F103) || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
    || defined(BOARD_MIBO48_STM32F303) \
    || defined(BOARD_MIBO64_STM32F405) \
    || defined(BOARD_MIBO48_STM32G473)

TGpioPin    pin_irqctrl(PORTNUM_A, 1, false);
TGpioPin    pin_extirq(PORTNUM_A, 0, false);

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  // On STM32 the first 16 external IRQs are tied to GPIO sources, where you can select the GPIO port
  extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);

  // Search the EXTI interrupt number from the MCU header definition (this can be found always at the beginning)
  #define EXTINT_IRQ_NUM                   6
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_06  // overrides the weak definition in vectors.cpp
}

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743) || defined(BOARD_NUCLEO_H723)

TGpioPin    pin_irqctrl(PORTNUM_G, 1, false);
TGpioPin    pin_extirq(PORTNUM_G, 0, false);

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  // On STM32 the first 16 external IRQs are tied to GPIO sources, where you can select the GPIO port
  extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);

  // Search the EXTI interrupt number from the MCU header definition (this can be found always at the beginning)
  #define EXTINT_IRQ_NUM                   6
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_06  // overrides the weak definition in vectors.cpp
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

TGpioPin  pin_extirq(  PORTNUM_C, 21, false);  // D9
TGpioPin  pin_irqctrl( PORTNUM_C, 22, false);  // D8

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);
  //extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING | HWEXTIRQ_FALLING); // test both edge

  // The shared PIOC IRQ will be used, warning this MCU requires IrqBegin()
  #define EXTINT_IRQ_NUM                  13
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_13  // overrides the weak definition in vectors.cpp
}

#elif defined(BOARD_XPLAINED_SAME70)

TGpioPin  pin_extirq(PORTNUM_B, 0, false);  // D15
TGpioPin  pin_irqctrl(PORTNUM_B, 1, false); // D14

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);
  //extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING | HWEXTIRQ_FALLING); // test both edge

  // The shared PIOB IRQ will be used, warning this MCU requires IrqBegin()
  #define EXTINT_IRQ_NUM                  11
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_11  // overrides the weak definition in vectors.cpp
}

#elif defined(BOARD_MIBO64_ATSAME5X)

TGpioPin  pin_extirq(PORTNUM_A, 0, false);
TGpioPin  pin_irqctrl(PORTNUM_A, 2, false);  // A1 = LED

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT | PINCFG_AF_A);  // the EXTINT[0]

  extirq.Init(0, HWEXTIRQ_RISING);  // setup EXTINT[0]

  // Search the EIC_x_IRQn interrupt number from the MCU header definition (this can be found always at the beginning)
  #define EXTINT_IRQ_NUM                  12
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_12  // overrides the weak definition in vectors.cpp
}

// RP

#elif defined(BOARD_RPI_PICO)

// for this test short the GPIO16 and GPIO17

TGpioPin  pin_extirq( 0, 17, false);
TGpioPin  pin_irqctrl(0, 16, false);

void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);

  // The RP2040 has one IRQ for all the GPIO pins
  // Search the IO_IRQ_BANK0 / IO_BANK0_IRQn interrupt number from the MCU header definition
  #define EXTINT_IRQ_NUM                  13
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_13
}

#else
  #error "External IRQ board specific setup is missing"
#endif


#if defined(EXTINT_IRQ_HANDLER)

#warning "latency measurement is unprecise because the time base is running at 12 MHz"

extern "C" void EXTINT_IRQ_HANDLER()
{
  g_extirq_accept_time = CLOCKCNT;
  ++g_extirq_cnt;

  // ATSAM MCUs require IrqBegin(), exactly once per IRQ handler
  // on ATSAM, the IrqBegin() acknowledges the IRQ as well, the IrqAck is empty
  extirq.IrqBegin();

  extirq.IrqAck();
  // The IRQ acknowledge might be delayed, and thus the IRQ will be triggered again, and the IRQ counter
  // is incremented twice. (e.g. this was observed on the STM32F746)
  // Issuing a read to the same peripheral ensures that the IRQ ACK finishes
  // when we leave the IRQ handler:
  if (extirq.IrqPending()) { }
}

#else
  #error "define the EXTINT_IRQ_HANDLER!"
#endif


void test_extirq()
{
  TRACE("External IRQ Test\r\n");

  init_extirq_pins();

  setup_irq(EXTINT_IRQ_NUM);

  unsigned hbclocks = SystemCoreClock * 1;  // IRQ every 2 s
  unsigned hbcounter = 0;

  unsigned t0, t1;

  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    if (g_extirq_cnt != g_extirq_cnt_prev)
    {
      unsigned latency = g_extirq_accept_time - g_extirq_start_time;
      TRACE("EXTIRQ-%i, latency: %u clocks\r\n", g_extirq_cnt, latency); // warning: this takes long, be careful with TRACE-es in IRQ handlers !

      g_extirq_cnt_prev = g_extirq_cnt;
    }

    if (t1-t0 > hbclocks)
    {
      ++hbcounter;

      // create the IRQ pulse

      if (hbcounter & 1)
      {
        g_extirq_start_time = CLOCKCNT;
        pin_irqctrl.Set1();  // the Set1(), Set0() is faster than the Toggle() on STM32
      }
      else
      {
        g_extirq_start_time = CLOCKCNT;
        pin_irqctrl.Set0();  // the Set1(), Set0() is faster than the Toggle() on STM32
      }

      // and finally do the usual led blinking

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

      board_show_hexnum(hbcounter);

      t0 = t1;
    }
  }
}

// ----------------------------------------------------------------------------
