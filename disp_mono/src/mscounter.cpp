/*
 * mscounter.cpp
 *
 *  Created on: Mar 4, 2022
 *      Author: vitya
 */

#include "platform.h"
#include "mscounter.h"

volatile unsigned g_mscounter = 0;

#ifdef MCU_ARMM

extern "C" void SysTick_Handler(void)
{
  ++g_mscounter;
}

void mscounter_init()
{
  SysTick_Config(SystemCoreClock / 1000);
}

#elif defined(MCU_RV32I)

  #if defined(MCUF_ESP)

  // light interrupt handler the interrupt attribute must be defined here
  // because no register saving happened before !
  extern "C" __attribute__((interrupt)) void IRQ_Handler_07()
  {
    ++g_mscounter;
    SYS_TIMER->INT_CLR |= (SYS_TIMER_TARGET0_INT_CLR);
  }

  void mscounter_init()
  {
    // Configure the System Timer for 1 ms period

    // the systimer has a fixed 16 MHz counting frequency
    uint32_t timer_period = (16000000 / 1000);

    SYSTEM->PERIP_CLK_EN0 |= SYSTEM_SYSTIMER_CLK_EN;  // this is not really necessary (enabled by default)

    SYS_TIMER->TARGET_CONF[0] = (0
      | (0 << 31) // UNIT_SEL: 0 = unit0, 1 = unit1
      | (1 << 30) // PERIOD_MODE: 1 = period mode
      | (timer_period << 0)
    );

    SYS_TIMER->COMP_LOAD[0] = 1; // actualize the load

    SYS_TIMER->CONF |= (SYS_TIMER_TARGET0_WORK_EN);
    SYS_TIMER->INT_ENA |= (SYS_TIMER_TARGET0_INT_ENA);

    // route the systimer interrupt to IRQ7
    *(uint32_t *)(INTERRUPT_CORE0_SYSTIMER_TARGET0_INT_MAP_REG) = 7;
    // set the priority level!
    *(uint32_t *)(INTERRUPT_CORE0_CPU_INT_PRI_7_REG) = 15;
    // enable this interrupt on the core 0
    *(uint32_t *)(INTERRUPT_CORE0_CPU_INT_ENABLE_REG) = (1 << 7);
  }

  #elif defined(MCUF_GD32V)

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
    #error "Unhandled RV32I 1 ms timer"
  #endif

#else
  #error "unhandled MCU for mscounter"
#endif
