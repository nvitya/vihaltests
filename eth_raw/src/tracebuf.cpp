/* -----------------------------------------------------------------------------
 * This file is a part of the VIHAL TEST project: https://github.com/nvitya/vihaltests
 * Copyright (c) 2021 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     tracebuf.cpp
 *  brief:    buffered trace to use real-time sensitive task tracing
 *            if the buffer is full then it goes to blocking mode!
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#include <stdio.h>
#include <stdarg.h>

#include "tracebuf.h"

#define TRACE_FMT_BUFFER_SIZE  256

TTraceBuf tracebuf;

void TTraceBuf::Init(THwUart * auart, char * abuf, unsigned alen)
{
	bufptr = abuf;
	bufsize = alen;
	wridx = 0;
	rdidx = 0;
	puart = auart;
}

void TTraceBuf::AddChar(char c)
{
	bufptr[wridx] = c;
	unsigned nextidx = wridx + 1;
	if (nextidx >= bufsize)
	{
		nextidx = 0;
	}

	if (waitsend)
	{
		wridx = nextidx;
		Flush();
	}
	else
	{
		while (nextidx == rdidx)  // go to wait mode if buffer full
		{
			Run();
		}
		wridx = nextidx;
	}
}

void TTraceBuf::printf(const char * fmt, ...)
{
  va_list arglist;
  va_start(arglist, fmt);

  printf_va(fmt, &arglist);

  va_end(arglist);
}

void TTraceBuf::printf_va(const char * fmt, va_list * arglist)
{
  char * pch;

  // allocate format buffer on the stack:
  char fmtbuf[TRACE_FMT_BUFFER_SIZE];

  pch = &fmtbuf[0];

  unsigned cnt = vsnprintf(pch, TRACE_FMT_BUFFER_SIZE, fmt, *arglist);
  while (cnt)
  {
  	AddChar(*pch++);
  	--cnt;
  }

  if (waitsend)  Flush();
}


void TTraceBuf::Run()
{
	if (rdidx != wridx)
	{
		if (puart->TrySendChar(bufptr[rdidx]))
		{
			++rdidx;
			if (rdidx >= bufsize)  rdidx = 0;
		}
	}
}

void TTraceBuf::Flush()
{
	while (rdidx != wridx)
	{
		Run();
	}
}
