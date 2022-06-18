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

#ifdef MCU_ARMM

void setup_irq(int airqnum)
{
  IRQn_Type irqnum = IRQn_Type(airqnum);
  NVIC_SetPriority(irqnum, IRQPRIO_EXTINT);
  NVIC_ClearPendingIRQ(irqnum);
  NVIC_EnableIRQ(irqnum);
}

#else

void setup_irq(int airqnum)
{
  #warning "implement non-arm IRQ handling"
}

#endif

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

#elif defined(BOARD_LONGAN_NANO)

TGpioPin    pin_irqctrl(PORTNUM_A, 1, false);
TGpioPin    pin_extirq(PORTNUM_A, 0, false);

#define TIMER_MSIP           0xFFC
#define TIMER_MSIP_size      0x4
#define TIMER_MTIMECMP       0x8
#define TIMER_MTIMECMP_size  0x8
#define TIMER_MTIME          0x0
#define TIMER_MTIME_size     0x8

#define TIMER_CTRL_ADDR           0xd1000000
#define TIMER_REG(offset)         _REG32(TIMER_CTRL_ADDR, offset)
#define TIMER_FREQ              ((uint32_t)SystemCoreClock/4)  //units HZ

#define CSR_MTVEC   0x305

#define ECLICINTCTLBITS  4

//ECLIC memory map
//   Offset
//  0x0000       1B          RW        ecliccfg
#define ECLIC_CFG_OFFSET            0x0
//  0x0004       4B          R         eclicinfo
#define ECLIC_INFO_OFFSET           0x4
//  0x000B       1B          RW        mintthresh
#define ECLIC_MTH_OFFSET            0xB
//
//  0x1000+4*i   1B/input    RW        eclicintip[i]
#define ECLIC_INT_IP_OFFSET            0x1000
//  0x1001+4*i   1B/input    RW        eclicintie[i]
#define ECLIC_INT_IE_OFFSET            0x1001
//  0x1002+4*i   1B/input    RW        eclicintattr[i]
#define ECLIC_INT_ATTR_OFFSET          0x1002
#define ECLIC_INT_CTRL_OFFSET          0x1003

#define ECLIC_INT_ATTR_SHV              0x01
#define ECLIC_INT_ATTR_TRIG_LEVEL       0x00
#define ECLIC_INT_ATTR_TRIG_EDGE        0x02
#define ECLIC_INT_ATTR_TRIG_POS         0x00
#define ECLIC_INT_ATTR_TRIG_NEG         0x04

//  0x1003+4*i   1B/input    RW        eclicintctl[i]
//
//  ...
//
#define ECLIC_ADDR_BASE           0xD2000000


void eclic_set_cliccfg(uint8_t cliccfg)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_CFG_OFFSET) = cliccfg;
}

uint8_t eclic_get_cliccfg()
{
  return *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_CFG_OFFSET);
}

void eclic_enable_interrupt(uint32_t source)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_IE_OFFSET+source*4) = 1;
}

void eclic_disable_interrupt(uint32_t source)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_IE_OFFSET+source*4) = 0;
}

void eclic_set_pending(uint32_t source)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_IP_OFFSET+source*4) = 1;
}

void eclic_clear_pending(uint32_t source)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_IP_OFFSET+source*4) = 0;
}

void eclic_set_intctrl(uint32_t source, uint8_t intctrl)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_CTRL_OFFSET+source*4) = intctrl;
}

uint8_t eclic_get_intctrl(uint32_t source)
{
  return *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_CTRL_OFFSET+source*4);
}

void eclic_set_intattr(uint32_t source, uint8_t intattr)
{
  *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_ATTR_OFFSET+source*4) = intattr;
}

uint8_t eclic_get_intattr(uint32_t source)
{
  return *(volatile uint8_t*)(ECLIC_ADDR_BASE+ECLIC_INT_ATTR_OFFSET+source*4);
}



void eclic_mode_enable()
{
  uint32_t mtvec_value = cpu_csr_read(CSR_MTVEC);
  mtvec_value = mtvec_value & 0xFFFFFFC0;
  mtvec_value = mtvec_value | 0x00000003; // sets non-vectored mode, harder service routine with interrupt preemptition by higher level
  cpu_csr_write(CSR_MTVEC, mtvec_value);
}

void eclic_set_nlbits(uint8_t nlbits)
{
  uint8_t cfgr = ECLIC->CFG;
  cfgr &= ~ECLIC_CFG_NLBITS_MASK;
  cfgr |= (nlbits << ECLIC_CFG_NLBITS_LSB);
  ECLIC->CFG = cfgr;
}

void eclic_int_set_level_prio(uint32_t intnum, uint8_t level_prio)
{
  uint8_t ctlr = ECLIC->INT[intnum].CTL;
  ctlr &= ~ECLIC_INT_CTL_MASK;
  ctlr |= (level_prio << ECLIC_INT_CTL_SHIFT);
  ECLIC->INT[intnum].CTL = ctlr;
}

void eclic_int_enable(uint32_t intnum)
{
  ECLIC->INT[intnum].IE = 1;
}

void eclic_int_disable(uint32_t intnum)
{
  ECLIC->INT[intnum].IE = 0;
}

bool eclic_int_enabled(uint32_t intnum)
{
  return (ECLIC->INT[intnum].IE != 0);
}

void systick_config(void)
{
  SYSTIMER->MTIMECMP64 = (SystemCoreClock >> 0);
  SYSTIMER->MTIME64 = 0;
}


void test_systimer()
{
  // ECLIC Prepare
  eclic_mode_enable();
  eclic_set_nlbits(ECLIC_NLBITS_LEVEL3_PRIO1);

  // IRQ Prepare
  ECLIC->INT[SYSTIMER_IRQn].ATTR = (0
    | (0 << 1)  // TRIG(2): 0 = level triggered, 1 = rising edge, 3 = falling edge
    | (0 << 0)  // SHV: 1 = vectored handling
  );
  eclic_int_set_level_prio(SYSTIMER_IRQn, 4);
  eclic_int_enable(SYSTIMER_IRQn);

  // settin up the periodic timer
  SYSTIMER->MTIMECMP64 = (SystemCoreClock >> 2);  // div 4 = 1s period
  SYSTIMER->MTIME64 = 0;
  SYSTIMER->MTIMECTL = (0
    | (0 << 2)  // CLKSRC: 0 = mtime_toggle_a, 1 = core_aon_clk
    | (1 << 1)  // CMPCLREN: 1 = auto clear the counter on compare  // does not seems to work
    | (0 << 0)  // TIMESTOP: 1 = stop timer
  );

  //ECLIC->MTH = 0;
  mcu_enable_interrupts();
}

extern "C" void IRQ_Handler_07() // SYSTIMER INT (number 7)
{
  SYSTIMER->MTIME32[0] = 0;  // restart the timer, this must be the very first line, but this is still not precise

  TRACE("System IRQ!\r\n");
}


void init_extirq_pins()
{
  pin_irqctrl.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_extirq.Setup(PINCFG_INPUT);

  // On STM32 the first 16 external IRQs are tied to GPIO sources, where you can select the GPIO port
  //extirq.Init(pin_extirq.portnum, pin_extirq.pinnum, HWEXTIRQ_RISING);

  // Search the EXTI interrupt number from the MCU header definition (this can be found always at the beginning)
  #define EXTINT_IRQ_NUM                   6
  #define EXTINT_IRQ_HANDLER  IRQ_Handler_06  // overrides the weak definition in vectors.cpp

  test_systimer();
}

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

#warning "latency measurement is unprecise because the time base is running at 12 MHz"

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

  unsigned hbclocks = SystemCoreClock * 1;  // every 1-2 s
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
