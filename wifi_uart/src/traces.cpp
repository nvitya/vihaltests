/*
 *  file:     traces.cpp
 *  brief:    Buffered trace message redirection to UART, init required !
 *  created:  2021-10-02
 *  authors:  nvitya
*/

#include "traces.h"

char trace_buffer[1024];

void traces_init()
{
  tracebuf.waitsend = false;  // the USB requires fast service !
	tracebuf.Init(&conuart, &trace_buffer[0], sizeof(trace_buffer));
}




