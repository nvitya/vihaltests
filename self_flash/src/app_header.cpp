// app_header.cpp

#include "platform.h"
#include "app_header.h"

extern unsigned __app_image_end;

extern "C" void cold_entry();

#if defined(MCUF_VRV100)

extern "C" void _cold_entry(void);

extern const bootblock_header_t application_header;  // required so that the optimizer keeps this

__attribute__((section(".application_header"),used))
const bootblock_header_t application_header =
{
  .signature = BOOTBLOCK_SIGNATURE,
  .length = unsigned(&__app_image_end) - unsigned(&application_header) - sizeof(bootblock_header_t),
  .addr_load = unsigned(&application_header),
  .addr_entry = (unsigned)_cold_entry,
  ._reserved10 = 0,
  .compid = 0xEEEEEEEE,  // will be updated later
  .csum_body = 0,
  .csum_head = 0
};


#endif


#if defined(BOARD_RPI_PICO)

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

#endif
