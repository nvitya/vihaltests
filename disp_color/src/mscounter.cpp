/*
 * mscounter.cpp
 *
 *  Created on: Mar 4, 2022
 *      Author: vitya
 */

#include "platform.h"
#include "mscounter.h"

volatile unsigned g_mscounter = 0;

extern "C" void SysTick_Handler(void)
{
  ++g_mscounter;
}

void mscounter_init()
{
  SysTick_Config(SystemCoreClock / 1000);
}
