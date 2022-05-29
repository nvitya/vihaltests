// app_header.cpp

#include "platform.h"
#include "app_header.h"

extern unsigned __image_end;

extern "C" void _startrom(void);

__attribute__((section(".application_header"),used))
const TAppHeader application_header =
{
  .signature = APP_HEADER_SIGNATURE,
	.copy_addr = 0x21000000,
	.copy_length = unsigned(&__image_end) - 0x21000000,
	.entry_point = (unsigned)_startrom
};


