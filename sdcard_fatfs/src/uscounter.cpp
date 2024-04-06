/*
 * uscounter.cpp
 *
 *  Created on: Apr 5, 2024
 *      Author: vitya
 */

#include <uscounter.h>

#include "stm32_utils.h"

TUsCounter  uscounter;

// NOTE: STM32 version only so far

bool TUsCounter::Init()
{
  // only TIM5 and TIM2 are 32-bit timers

  if (5 == timerdev)
  {
    regs = TIM5;
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
  }
  else
  {
    regs = TIM2;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  }

  uint32_t periphclock = stm32_bus_speed(STM32_BUSID_APB1);   // usually CPUCLOCK / 4

  cnt64_high = 0;
  cnt32_prev = 0;

  regs->CR1 = 0; // stop
  regs->CR2 = 0;
  regs->PSC = (periphclock / 1000000) - 1;  // for 1 us clock
  //regs->CNT = 0; // reset the counter
  regs->CR1 = 1; // enable the timer
  regs->EGR = 1; // reinit, start the timer (maybe not required?)

  initialized = true;
  return true;
}

uint64_t TUsCounter::Get64()
{
  uint32_t cnt32_cur = Get32();
  if (cnt32_cur < cnt32_prev)
  {
    // the 32-bit version overflowed (after 66 minutes)
    cnt64_high += 0x100000000ull;
  }
  cnt32_prev = cnt32_cur;
  return cnt64_high + cnt32_cur;
}
