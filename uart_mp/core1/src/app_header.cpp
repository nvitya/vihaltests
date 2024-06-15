// app_header.cpp

#include "platform.h"
#include "app_header.h"
#include "multicore_app.h"

extern unsigned __app_image_end;

extern "C" void cold_entry();

#if defined(MCUF_RP) // RP2040

extern const TAppHeader application_header;  // this is required, otherwise it will be removed by the linker


__attribute__((section(".application_header"),used))
const TAppHeader application_header =
{
  .signature = APP_HEADER_SIGNATURE,
  .length = unsigned(&__app_image_end) - SECONDARY_CODE_ADDR,
	.addr_load = SECONDARY_CODE_ADDR,
	.addr_entry = (unsigned)cold_entry,

	.customdata = 0,  // will be set to SECONDARY_SELF_FLASH_FLAG
	.compid = 0,
	.csum_body = 0,
	.csum_head = 0
};

#endif
