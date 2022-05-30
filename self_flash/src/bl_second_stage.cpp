/*
 * bl_second_stage.cpp
 *
 *  Created on: May 29, 2022
 *      Author: vitya
 */

#include "platform.h"
#include "app_header.h"

#pragma GCC optimize ("Os")

// this code is targeted to SRAM5 (0x20041000), the RP2040 boot rom loads this code there

static ssi_hw_t * const ssi = (ssi_hw_t *) XIP_SSI_BASE;

__attribute__((section(".bl_second_stage"),used))
static void flash_read(unsigned flashaddr, uint8_t * dst, unsigned len)
{
  // Disable SSI for further config
  ssi->ssienr = 0;
  // Clear sticky errors (clear-on-read)
  (void) ssi->sr;
  (void) ssi->icr;
  // Hopefully-conservative baud rate for boot and programming

  uint32_t cr0 = (0
    | (0  << 24)  // SSTE: slave select toggle
    | (0  << 21)  // SPI_FRF(2): 0 = single, 1 = dual, 2 = quad
    | (7  << 16)  // DFS_32(5): 31 = 32 bit frames
    | (0  << 12)  // CFS(4): control frame size
    | (0  << 11)  // SRL: shift register loop
    | (0  << 10)  // SLV_OE: slave output enable (???, not well documented)
    | (3  <<  8)  // TMOD(2): 0 = transmit+receive, 3 = eeprom mode: RX starts after control data
    | (0  <<  7)  // SCPOL serial clock polarity
    | (0  <<  6)  // SCPH: serial clock phase
    | (0  <<  4)  //   FRF(2): ??? not documented, the SDK and the BOOTROM uses only the SPI_FRF field
    | (0  <<  0)  //   DFS(4): ??? not documented, the SDK and the BOOTROM uses only the DFS_32 field
  );

  uint32_t spi_cr0 = (0
    | (0  << 24)  // XIP_CMD(8): instruction code in XIP mode when INST_L = 8-bit
    | (0  << 18)  // SPI_RXDS_EN: read data strobe enable
    | (0  << 17)  // INST_DDR_EN
    | (0  << 16)  // SPI_DDR_EN
    | (0  << 11)  // WAIT_CYCLES(5)
    | (2  <<  8)  // INST_L(2): 0 = no instruction, 2 = 8-bit instruction, 3 = 16-bit
    | (6  <<  2)  // ADDR_L(4): in 4 bit increments
    | (0  <<  0)  // TRANS_TYPE(2): 0 = all single, 1 = cmd single and addr multi-line, 2 = cmd and address multi-line
  );

  ssi->ctrlr0 = cr0;
  ssi->spi_ctrlr0 = spi_cr0;
  ssi->dmacr = 0;
  ssi->ctrlr1 = len-1;  // ONE LESS !

  // Slave selected when transfers in progress
  ssi->ser = 1;

  // Re-enable
  ssi->ssienr = 1;

  // send command and address
  ssi->dr0 = 0x03; // read
  ssi->dr0 = (flashaddr >> 16) & 0xFF;
  ssi->dr0 = (flashaddr >>  8) & 0xFF;
  ssi->dr0 = (flashaddr >>  0) & 0xFF;

  while (len)
  {
    if (ssi->sr & SSI_SR_RFNE_BITS)
    {
      *dst++ = ssi->dr0;
      --len;
    }
  }

  ssi->ssienr = 0; // ...
}

typedef void (* TEntryFunc)(void);

extern "C" __attribute__((section(".bl_second_stage_start"),noreturn,used))
void bl_second_stage_start()
{
  TAppHeader apph;

  // initialize the SSI
  ssi->ssienr = 0;  // disable the SSI for reconfigure
  ssi->baudr =  4;

  flash_read(0x001000, (uint8_t *)&apph, sizeof(apph)); // application header at 4k

  if (APP_HEADER_SIGNATURE == apph.signature)
  {
    flash_read(0x002000, (uint8_t *)apph.copy_addr, apph.copy_length);

    TEntryFunc entryfunc = TEntryFunc(apph.entry_point);
    (*entryfunc)();
  }

  // stay here forever...
  while (true)
  {
    __NOP();
  }

}
