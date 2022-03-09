/*
 *  file:     startup_armm.cpp
 *  brief:    Entry point for ARM Cortex-M Processors
 *  creted:   2021-10-23
 *  authors:  nvitya
 *  notes:
 *    The ARMM platform does not have Assembly initialization part, so the two reset entry points
 *    are provided here.
 *    This code is separated from the VIHAL core, because some targets might require
 *    special adjustments (like SDRAM preparation)
*/

#include "platform.h"

#ifdef MCU_ARMM

#include "cppinit.h"

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing);

extern "C" __attribute__((section(".startup"), used, noreturn))
void soft_entry()  // used at development of RAM-loaded (self-flashing) applications
{
  mcu_preinit_code(); // inline code for preparing the MCU, RAM regions. Without this even the stack does not work on some MCUs.

  // the stack might not be set properly so set it
  asm("ldr  r0, =__stack");
  asm("mov  sp, r0");

  memory_region_setup();  // copy code to ram, initialize .data, zero .bss sections

  _start(1);
}

extern "C" __attribute__((section(".startup"), used, noreturn))
void cold_entry()  // power on start using integrated bootloaders and non volatile flash memory
{
  mcu_preinit_code(); // inline code for preparing the MCU, RAM regions. Without this even the stack does not work on some MCUs.

  // the stack might not be set properly so set it
  asm("ldr  r0, =__stack");
  asm("mov  sp, r0");

  memory_region_setup();  // copy code to ram, initialize .data, zero .bss sections

  _start(0);
}

#endif // ifdef MCU_ARMM


