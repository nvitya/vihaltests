/*
 * uscounter.h
 *
 *  Created on: Apr 5, 2024
 *      Author: vitya
 */

#ifndef SRC_USCOUNTER_H_
#define SRC_USCOUNTER_H_

#include "platform.h"

// NOTE: STM32 version only so far

class TUsCounter
{
public:
  bool            initialized = false;
  TIM_TypeDef *   regs = nullptr;
  int             timerdev = 2;

  bool Init();

  inline uint32_t Get32()
  {
    return regs->CNT;
  }

  uint64_t Get64(); // must be called at least every half hour

protected:
  uint64_t        cnt64_high = 0;
  uint32_t        cnt32_prev = 0;
};

extern TUsCounter  uscounter;

#endif /* SRC_USCOUNTER_H_ */
