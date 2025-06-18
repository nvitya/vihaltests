/*
 *  file:     board_pins.cpp
 *  brief:    VGBOOT IMXRT1020-EVK Board pins
 *  date:     2023-09-16
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "hwsdram.h"
#include "traces.h"

THwUart   conuart;  // console uart
unsigned  pin_led_count = 1;

THwQspi   fl_qspi;

TGpioPin  pin_led[MAX_LEDS] = { TGpioPin(), TGpioPin(), TGpioPin(), TGpioPin() };
TGpioPin  pin_user_button(5, 0, false);   // = WAKEUP PIN

bool flexram_config_ok()
{
  unsigned tmp = IOMUXC_GPR->GPR16;
  if (tmp & (1 << 2))
  {
    if (IOMUXC_GPR->GPR17 == FLEXRAM_CONFIG)
    {
      return true;
    }
    else if (IOMUXC_GPR->GPR17 == FLEXRAM_CONFIG_REBOOT)  // special config for internal reboots which gives the
                                                          // internal boot rom 32 k OTC RAM
    {
      IOMUXC_GPR->GPR17 = FLEXRAM_CONFIG; // turn off the OTC RAM again
      return true;
    }
  }

  return false;
}

void flexram_init()
{
  unsigned tmp;

  TRACE("FlexRAM: ITC=%uk, DTC=%uk, ORAM=%uk\r\n", (FLEXRAM_ITC_SIZE >> 10), (FLEXRAM_DTC_SIZE >> 10), (FLEXRAM_OCR_SIZE >> 10));

  if (flexram_config_ok())
  {
    return;
  }

  TRACE("Updating FlexRAM config: %04X -> %04X\r\n", IOMUXC_GPR->GPR17, FLEXRAM_CONFIG);

  // it is in default configuration, reconfigure

  IOMUXC_GPR->GPR17 = FLEXRAM_CONFIG;
  IOMUXC_GPR->GPR16 |= (1 << 2); // use the flexram config

  // change the ITCM size
  tmp = IOMUXC_GPR->GPR14;
  tmp &= ~(0xF << 16);
  tmp |=  (0xA << 16);     // 0xA = 512 k. We don't have so much so the end must not be addressed
  IOMUXC_GPR->GPR14 = tmp;
}

void board_pins_init()
{
  pin_led_count = 1;
  pin_led[0].Assign(1, 8, false);  // GPIO_AD_B0_08 = GPIO_1_8
  pin_led[0].Setup(PINCFG_OUTPUT | PINCFG_GPIO_INIT_0);

  pin_user_button.Setup(PINCFG_INPUT | PINCFG_PULLUP);

  // Console UART
  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_12_LPUART1_TX, 0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, 0);
  conuart.baudrate = CONUART_UART_SPEED;
  conuart.Init(1); // UART1

  // QSPI: has internal pin setup in Init()

  // SDRAM pins
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_00_SEMC_DATA00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_01_SEMC_DATA01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_02_SEMC_DATA02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_03_SEMC_DATA03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_04_SEMC_DATA04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_05_SEMC_DATA05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_06_SEMC_DATA06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_07_SEMC_DATA07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_08_SEMC_DM00,    0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_09_SEMC_ADDR00,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_10_SEMC_ADDR01,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_11_SEMC_ADDR02,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_12_SEMC_ADDR03,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_13_SEMC_ADDR04,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_14_SEMC_ADDR05,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_15_SEMC_ADDR06,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_16_SEMC_ADDR07,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_17_SEMC_ADDR08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_18_SEMC_ADDR09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_19_SEMC_ADDR11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_20_SEMC_ADDR12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_21_SEMC_BA0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_22_SEMC_BA1,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_23_SEMC_ADDR10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_24_SEMC_CAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_25_SEMC_RAS,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_26_SEMC_CLK,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_27_SEMC_CKE,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_28_SEMC_WE,      0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_29_SEMC_CS0,     0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_30_SEMC_DATA08,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_31_SEMC_DATA09,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_32_SEMC_DATA10,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_33_SEMC_DATA11,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_34_SEMC_DATA12,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_35_SEMC_DATA13,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_36_SEMC_DATA14,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_37_SEMC_DATA15,  0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_38_SEMC_DM01,    0);
  hwpinctrl.PadSetup(IOMUXC_GPIO_EMC_39_SEMC_DQS,     0);
}

void board_init()
{
  // Increase the ITC size
  flexram_init();

  // config for MT48LC16M16A2-6A: 32 MByte

  hwsdram.row_bits = 13;
  hwsdram.column_bits = 9;
  hwsdram.bank_count = 4;
  hwsdram.cas_latency = 3;

  hwsdram.row_precharge_delay = 3;
  hwsdram.row_to_column_delay = 3;
  hwsdram.recovery_delay = 2;
  hwsdram.row_cycle_delay = 9;

  hwsdram.burst_length = 8;

  hwsdram.Init();

  // initialize QSPI Flash

  fl_qspi.speed = QSPI_SPEED;
  fl_qspi.rxdmachannel = DMACH_QSPI_RX;
  fl_qspi.txdmachannel = DMACH_QSPI_TX;
  fl_qspi.multi_line_count = 4;
  fl_qspi.Init();

  spiflash.spi = nullptr;
  spiflash.qspi = &fl_qspi;
  spiflash.has4kerase = true;
}

bool board_app_start_inhibited()
{
  if (0 == pin_user_button.Value())  // user button pushed ?
  {
    return true;
  }
  else
  {
    return false;
  }
}

