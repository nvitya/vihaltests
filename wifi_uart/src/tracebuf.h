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
 *  file:     tracebuf.h
 *  brief:    buffered trace to use real-time sensitive task tracing
 *            if the buffer is full then it goes to blocking mode!
 *  date:     2021-11-18
 *  authors:  nvitya
*/

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
