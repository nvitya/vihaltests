/*
 *  file:     traces.cpp
 *  brief:    Buffered trace message redirection to UART, init required !
 *  created:  2021-10-02
 *  authors:  nvitya
*/

#include "traces.h"

char trace_buffer[4096];

void traces_init()
{
#if 0 // defined(BOARD_VRV100_443)
  tracebuf.waitsend = true;  // has big internal fifos
#else
  tracebuf.waitsend = false;
#endif
	tracebuf.Init(&conuart, &trace_buffer[0], sizeof(trace_buffer));
}




