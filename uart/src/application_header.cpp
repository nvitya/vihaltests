// application_header.cpp

#include "platform.h"

#if defined(MCUF_VRV100)

extern unsigned __app_image_end;

extern "C" void _reset_entry(void);

extern const bootblock_header_t application_header;  // required so that the optimizer keeps this

__attribute__((section(".application_header"),used))
const bootblock_header_t application_header =
{
  .signature = BOOTBLOCK_SIGNATURE,
	.length = unsigned(&__app_image_end) - unsigned(&application_header) - sizeof(bootblock_header_t),
	.addr_load = unsigned(&application_header),
	.addr_entry = (unsigned)_reset_entry,
	._reserved10 = 0,
	.compid = 0xEEEEEEEE,  // will be updated later
	.csum_body = 0,
	.csum_head = 0
};


#endif
