// tracebuf.cpp

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
		while (nextidx == rdidx)
		{
			Run();  // go to blocking mode
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
