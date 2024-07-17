/*
 *  file:     startup_armr.cpp
 *  brief:    Entry point for ARM Cortex-R Processors
 *  creted:   2024-06-16
 *  authors:  nvitya
*/

#include "platform.h"

#ifdef MCU_ARMR

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

#endif // ifdef MCU_ARMR


