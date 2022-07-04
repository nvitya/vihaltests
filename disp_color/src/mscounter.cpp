/*
 * mscounter.cpp
 *
 *  Created on: Mar 4, 2022
 *      Author: vitya
 */

#include "platform.h"
#include "mscounter.h"

volatile unsigned g_mscounter = 0;

#if defined(CPU_ARMM)  // common solution for all ARM Cortex-M

extern "C" void SysTick_Handler(void)
{
  ++g_mscounter;
}

void mscounter_init()
{
  SysTick_Config(SystemCoreClock / 1000);
}

#elif defined(BOARD_LONGAN_NANO)

extern "C" void IRQ_Handler_07() // SYSTIMER INT (number 7)
{
  SYS_TIMER->MTIME64 = 0;  // restart the timer, this must be the very first line,
                          // but so the timer is still not so precise as a self-clearing one.
  ++g_mscounter;
}

void mscounter_init()
{
  mcu_irq_enable(SYSTIMER_IRQn);

  // settin up the periodic timer
  SYS_TIMER->MTIMECMP64 = (SystemCoreClock / 4000);  // this timer counts every 4th system clock
  SYS_TIMER->MTIME64 = 0;
  SYS_TIMER->MTIMECTL = (0
    | (0 << 2)  // CLKSRC: 0 = mtime_toggle_a, 1 = core_aon_clk
    | (1 << 1)  // CMPCLREN: 1 = auto clear the counter on compare  // does not seems to work
    | (0 << 0)  // TIMESTOP: 1 = stop timer
  );

}

#else
  #error "unhandled mscounter !"
#endif
