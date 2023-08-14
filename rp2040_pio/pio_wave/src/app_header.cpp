// app_header.cpp: this version is for RP2040 only

#include "platform.h"
#include "app_header.h"

extern unsigned __app_image_end;

extern "C" void cold_entry();

extern const TAppHeader application_header;  // this is required, otherwise it will be removed by the linker

__attribute__((section(".application_header"),used))
const TAppHeader application_header =
{
  .signature = APP_HEADER_SIGNATURE,
  .length = unsigned(&__app_image_end) - 0x21000000,
	.addr_load = 0x21000000,
	.addr_entry = (unsigned)cold_entry,

	.customdata = 0,
	.compid = 0,
	.csum_body = 0,
	.csum_head = 0
};

