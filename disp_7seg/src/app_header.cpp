// app_header.cpp

#include "platform.h"
#include "app_header.h"

extern unsigned __app_image_end;

extern "C" void cold_entry();

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

#if defined(MCUF_ESP)

// a valid (second stage bootloader) example at flash address 0:
//   E9 03 02 20 00 E0 3C 40 EE 00 00 00 05 00 00 00
//   00 00 00 00 00 00 00 00 00 61 CD 3F 8C 03 00 00

extern "C" void _cold_entry(void);

extern const vihal_esp_image_header_t  application_header;  // this is required, otherwise it will be removed by the linker

__attribute__((section(".application_header"), used))
const vihal_esp_image_header_t  application_header =
{
  .magic          = 0xE9,      // Magic word ESP_IMAGE_HEADER_MAGIC = 0xE9
  .segment_count  = 1,         // Count of memory segments
  .spi_mode       = 2,         // flash read mode, 2 = DIO
  .spi_speed_size = 0x20,      // low 4 bits: speed, 0 = 40 MHz;  high 4 bits: size, 2 = 4MB
  .entry_addr     = (unsigned)_cold_entry, // Entry address
  .wp_pin         = 0xEE,      // 0xEE = disabled
  .spi_pin_drv    = {0, 0, 0}, // Drive settings for the SPI flash pins (read by ROM bootloader)
  .chip_id        = 0x0005,    // Chip identification number, 0x0005 = ESP32c3
  .min_chip_rev   = 0,         // Minimum chip revision supported by image
  .reserved       = {0, 0, 0, 0, 0, 0, 0, 0},
  .hash_appended  = 0,         // If 1, a SHA256 digest "simple hash" (of the entire image) is appended after the checksum.

  .load_addr      = unsigned(&application_header) + sizeof(vihal_esp_image_header_t),  // load address of the first segment
  .data_len       = unsigned(&__app_image_end) - unsigned(&application_header) - sizeof(vihal_esp_image_header_t) // size of the first segment
};

// FOOTER:
//   The file is padded with zeros until its size is one byte less than a multiple of 16 bytes.
//   A last byte (thus making the file size a multiple of 16) is the checksum of the data of all segments.
//   The checksum is defined as the xor-sum of all bytes and the byte 0xEF.

#endif
