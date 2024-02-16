/*
 * test_soft_spi.cpp
 *
 *  Created on: Dec 2, 2023
 *      Author: vitya
 */

#include "soft_spi.h"
#include "hwpins.h"
#include "traces.h"

TSoftSpi spi;

#if defined(BOARD_EVK_IMXRT1020)

/*

J17
1 / D0 | GPIO_AD_B1_09 |                 | soft-spi-SCK
2 / D1 | GPIO_AD_B1_08 | SPI3-CS2-D2     | soft-spi-MOSI
3 / D2 | GPIO_AD_B0_09 |                 | !ENET-RXD1!
4 / D3 | GPIO_AD_B0_07 |                 | !DBG UART RX!
5 / D4 | GPIO_AD_B0_05 |                 | !BOARD LED!
6 / D5 | GPIO_AD_B0_06 |                 | !DBG UART TX!
7 / D6 | GPIO_AD_B0_14 |                 | !ENET-TXD0!
8 / D7 | GPIO_AD_B1_06 |                 | soft-spi-CS0

J18
1 / A0 | GPIO_AD_B1_10 | UART4-TX        | !USB-OTG-PWR!
2 / A1 | GPIO_AD_B1_11 | UART4-RX        | !USB-OTG-ID!
3 / A2 | GPIO_AD_B1_12 | SPI3-SCK        | !USB-OTG-OC! ADC-2 data - SCK
4 / A3 | GPIO_AD_B1_13 | SPI3-CS0        | ADC-2 data - ODR
5 / A4 | GPIO_AD_B1_15 | SPI3-MISO       | ADC-2 data - D0
6 / A5 | GPIO_AD_B1_14 | SPI3-MOSI       |

J19
1 / D8  | GPIO_AD_B1_07 | SPI3-CS3-D3    | !SD-VSELECT! - soft-spi-MISO
2 / D9  | GPIO_AD_B0_15 |                | !ENET-TXD1!
3 / D10 | GPIO_AD_B0_11 | SPI1-CS0       | !ENET-RX-EN!
4 / D11 | GPIO_AD_B0_12 | SPI1-MOSI      | !ENET-RX-ER!
5 / D12 | GPIO_AD_B0_13 | SPI1-MISO      | !ENET-TX-EN!
6 / D13 | GPIO_AD_B0_10 | SPI1-SCK       | !ENET-RXD0!
7 / GND | GND           |
8 / AREF| 3V3           |
9 / D14 | GPIO_SD_B1_03 | I2C4-SDA       |
10/ D15 | GPIO_SD_B1_02 | I2C4-SCL       |

J20
1 | N.C
2 | 3.3V
3 | RESET
4 | 3.3V
5 | 5V
6 | GND
7 | GND
8 | VIN (5V)

*/

TGpioPin  pin_cs;
TGpioPin  pin_clk;
TGpioPin  pin_mosi;
TGpioPin  pin_miso;

void soft_spi_init_pins()
{
  //hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B1_06_GPIO1_IO22, PINCFG_OUTPUT | PINCFG_GPIO_INIT_1); // CS
  pin_cs.Assign(1, 22, false);
  pin_cs.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_1);
  pin_cs.Set1();
  pin_cs.Set0();
  pin_cs.Set1();

  //hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B1_09_GPIO1_IO25, PINCFG_OUTPUT); // SCK
  pin_clk.Assign(1, 25, false);
  pin_clk.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);
  pin_clk.Set1();
  pin_clk.Set0();
  pin_clk.Set1();
  pin_clk.Set0();

  //hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B1_08_GPIO1_IO24, PINCFG_OUTPUT); // MOSI
  pin_mosi.Assign(1, 24, false);
  pin_mosi.Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  //hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B1_07_GPIO1_IO23, PINCFG_INPUT | PINCFG_PULLUP); // MISO
  pin_miso.Assign(1, 23, false);
  pin_miso.Setup(PINCFG_INPUT | PINCFG_PULLUP);
}

#else
  #error "unhandled board!"
#endif

uint8_t txbuf[1024];
uint8_t rxbuf[1024];

void show_mem(void * addr, unsigned len)
{
  unsigned char * cp = (unsigned char *)addr;
  TRACE("Dumping memory at %08X, len = %u\r\n", addr, len);
  for (unsigned n = 0; n < len; ++n)
  {
    TRACE(" %02X", *cp++);
    if (n % 16 == 15) TRACE("\r\n");
  }
  TRACE("\r\n");
}

uint8_t read_adc_reg(uint8_t areg)
{
  uint8_t ltx[2];
  uint8_t lrx[2];

  ltx[0] = 0x80 + areg;
  ltx[1] = 0x00;

  spi.Transfer(0, 0, 0, 2, &ltx[0], &lrx[0]);

  TRACE("ADC-READ(%02X) -> %02X\r\n", areg, lrx[1]);

  return lrx[1];
}

void write_adc_reg(uint8_t areg, uint8_t avalue)
{
  uint8_t ltx[2];
  uint8_t lrx[2];

  ltx[0] = 0x00 + areg;
  ltx[1] = avalue;

  spi.Transfer(0, 0, 0, 2, &ltx[0], &lrx[0]);

  TRACE("ADC-WRITE(%02X) <- %02X\r\n", areg, avalue);
}

void test_soft_spi()
{
  TRACE("Testing Soft-SPI...\r\n");

  soft_spi_init_pins();

  spi.pin_cs[0] = &pin_cs;
  spi.pin_clk = &pin_clk;
  spi.pin_mosi = &pin_mosi;
  spi.pin_miso = &pin_miso;
  if (!spi.Init())
  {
    TRACE("SPI INIT ERROR!");
    return;
  }
  else
  {
    TRACE("SPI init ok.\r\n");
  }

  unsigned dlen = 64;
  unsigned n;

#if 0

  for (n = 0; n < dlen; ++n)
  {
    txbuf[n] = 0xD1 + n;
  }

  TRACE("Testing SPI with %u data...\r\n", dlen);

  spi.Transfer(0xCB, 0x010203, SPITR_CMD1 | SPITR_ADDR3, dlen, &txbuf[0], &rxbuf[0]);

  TRACE("Finished, results:\r\n");
  show_mem(&rxbuf[0], dlen);

#endif

#if 0
  // test AD4134 comm
  txbuf[0] = 0x80 + 0x0C;  // read of reg 0x0C
  txbuf[1] = 0x00;

  TRACE("RQ  -> %02X %02X\r\n", txbuf[0], txbuf[1]);
  spi.Transfer(0, 0, 0, 2, &txbuf[0], &rxbuf[0]);
  TRACE("ANS <- %02X %02X\r\n", rxbuf[0], rxbuf[1]);

#endif

#if 0
  read_adc_reg(0x0C);
  read_adc_reg(0x0D);
  TRACE("---\r\n");

  for (n = 0; n < 0x27; ++n)
  {
    read_adc_reg(n);
  }

  write_adc_reg(0x0A, 0x19);
  read_adc_reg(0x0A);

#endif

#if 1
  // test eeprom comm
  txbuf[0] = 0x00;
  txbuf[1] = 0x00;
  txbuf[2] = 0x00;
  txbuf[3] = 0x00;
  txbuf[4] = 0x00;

  TRACE("EEPROM ID CHECK...\r\n");

  spi.Transfer(0x9F, 0, SPITR_CMD1, 5, &txbuf[0], &rxbuf[0]);

  show_mem(&rxbuf[0], 5);

  // should result:  29 CC 00 01 00

#endif
  //pin_cs.Assign(aportnum, apinnum, ainvert)

  TRACE("Test Finished.\r\n");
}
