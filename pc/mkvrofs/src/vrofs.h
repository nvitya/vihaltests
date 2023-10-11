/* -----------------------------------------------------------------------------
 * VROFS - Read Only File System for Embedded Systems
 * Copyright (c) 2023 Viktor Nagy, nvitya
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
 *  file:     vrofs.h
 *  brief:    Main VROFS Header and Specification
 *  created:  2023-10-11
 *  authors:  nvitya
*/

#ifndef SRC_VROFS_H_
#define SRC_VROFS_H_

#include "stdint.h"

/* VROFS Specification

The VROFS consists of the following parts:
  - Main header: 32 Bytes
  - Index: file count * 80 Bytes
  - Data Block: variable length, file data padded to 8 Bytes

*/

#define VROFS_ID_10  "VROFSV10"

typedef struct
{
	char       vrofsid[8];    // "VROFSV10"
	uint32_t   headsize;      // in Bytes, including the 8-byte vrofsid
	uint32_t   indexsize;     // in Bytes
	uint32_t   datasize;      // in Bytes
	uint32_t   _reserved[3];  // all 0xFF-s
//
} TVrofsMainHead;  // 32 Bytes

typedef struct
{
	char       path[64];      // zero terminated string
	uint32_t   size;          // in Bytes
	uint32_t   offset;        // in Bytes, offset within the data block. The data start is 8-byte aligned
	uint32_t   _reserved[2];  // all 0xFF-s
//
} TVrofsIndexRec; // 80 Bytes


#endif /* SRC_VROFS_H_ */
