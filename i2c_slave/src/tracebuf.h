// tracebuf.h

#ifndef COMMON_TRACEBUF_H_
#define COMMON_TRACEBUF_H_

#include "stdarg.h"
#include "hwuart.h"

class TTraceBuf
{
public:
	char *     bufptr = nullptr;
	unsigned   bufsize = 0;
	unsigned   wridx = 0;
	unsigned   rdidx = 0;
	THwUart *  puart = nullptr;

	bool       waitsend = true;

	void Init(THwUart * auart, char * abuf, unsigned alen);

	void AddChar(char c);

	void printf(const char * fmt, ...);
	void printf_va(const char * fmt, va_list * arglist);

	void Run();
	void Flush();
};

extern TTraceBuf  tracebuf;

#endif /* COMMON_TRACEBUF_H_ */
