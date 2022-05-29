
#ifndef APP_HEADER_H_
#define APP_HEADER_H_

#include "stdint.h"

#define APP_HEADER_SIGNATURE  0xAA559966

typedef struct TAppHeader
{
   uint32_t signature;
   uint32_t copy_addr;
   uint32_t copy_length;
   uint32_t entry_point;
//
} TAppHeader;  // 16 bytes

#endif
