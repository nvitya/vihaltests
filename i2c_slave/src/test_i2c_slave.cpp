/*
 *  file:     test_i2c_slave.cpp
 *  brief:    I2C Slave Test (EEPROM emulation)
 *  version:  1.00
 *  date:     2021-11-07
 *  authors:  nvitya
*/

#include "platform.h"
#include "hwpins.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "i2c_eeprom_app.h"
#include "board_pins.h"

#include "traces.h"

#define I2C_ADDRESS  0x50  // EEPROM Emulation

TI2cEepromApp  i2capp;

void setup_irq(int airqnum)
{
  IRQn_Type irqnum = IRQn_Type(airqnum);
  NVIC_SetPriority(irqnum, IRQPRIO_I2C);
  NVIC_ClearPendingIRQ(irqnum);
  NVIC_EnableIRQ(irqnum);
}

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743)

void init_i2c_pins()
{
  // I2C1, CN7[2,4]
  hwpinctrl.PinSetup(PORTNUM_B, 8, PINCFG_AF_4 | PINCFG_OPENDRAIN);  // CN7[2] = I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B, 9, PINCFG_AF_4 | PINCFG_OPENDRAIN);  // CN7[4] = I2C1_SDA

  i2capp.devnum = 1;

  #define I2C_IRQ_NUM                   31
  #define I2C_IRQ_HANDLER   IRQ_Handler_31
}

#elif defined(BOARD_MIN_F103)  \
      || defined(BOARD_MIBO48_STM32F303)

void init_i2c_pins()
{
  // I2C1
  // open drain mode have to be used, otherwise it won't work
  // External pull-ups are required !
  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_AF_0 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_AF_0 | PINCFG_OPENDRAIN | PINCFG_SPEED_FAST); // I2C1_SDA

  i2capp.devnum = 1;

  #define I2C_IRQ_NUM                   31
  #define I2C_IRQ_HANDLER   IRQ_Handler_31
}

#elif  defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
      || defined(BOARD_MIBO64_STM32F405)

void init_i2c_pins()
{
  // I2C1
  // open drain mode have to be used, otherwise it won't work
  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA

  i2capp.devnum = 1;

  #define I2C_IRQ_NUM                   31
  #define I2C_IRQ_HANDLER   IRQ_Handler_31
}

#elif defined(BOARD_MIBO48_STM32G473)

void init_i2c_pins()
{
  // I2C1
  // open drain mode have to be used, otherwise it won't work

  // WARNING: the B6 pin did not work (was always low after enabling the AF_4)
  //          this is due the USB-C power delivery pull down functionality
  //          turning off did not work for me this way:
  //  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
  //  PWR->CR3 |= PWR_CR3_UCPD_DBDIS;
  //  hwpinctrl.PinSetup(PORTNUM_B,  6, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
  //  hwpinctrl.PinSetup(PORTNUM_B,  7, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA

  // Warning: PB8 = boot pin ! must be high at reset otherwise the Flash code does not start
  hwpinctrl.PinSetup(PORTNUM_B,  8, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SCL
  hwpinctrl.PinSetup(PORTNUM_B,  9, PINCFG_OUTPUT | PINCFG_AF_4 | PINCFG_OPENDRAIN); // I2C1_SDA

  i2capp.devnum = 1;

  #define I2C_IRQ_NUM                   31
  #define I2C_IRQ_HANDLER   IRQ_Handler_31
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void init_i2c_pins()
{
  #define TWI_NUM  0

  #if 0 == TWI_NUM
    // TWI0 (I2C0), the first two pins closest to the reset button
    hwpinctrl.PinSetup(PORTNUM_A, 17, PINCFG_AF_A);  // TWI0_SDA (TWD)
    hwpinctrl.PinSetup(PORTNUM_A, 18, PINCFG_AF_A);  // TWI0_SCL (TWCK)
    i2capp.devnum = 0;

    #define I2C_IRQ_NUM                   22
    #define I2C_IRQ_HANDLER   IRQ_Handler_22

  #elif 1 == TWI_NUM
    // TWI1 (I2C1), the pins 20, 21
    hwpinctrl.PinSetup(PORTNUM_B, 12, PINCFG_AF_A);  // TWI1_SDA (TWD)
    hwpinctrl.PinSetup(PORTNUM_B, 13, PINCFG_AF_A);  // TWI1_SCL (TWCK)

    i2capp.devnum = 1;

    #define I2C_IRQ_NUM                   23
    #define I2C_IRQ_HANDLER   IRQ_Handler_23
  #else
    #error "invalid TWI / I2C"
  #endif
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void init_i2c_pins()
{
  // SERCOM4
  hwpinctrl.PinSetup(PORTNUM_B,  8, PINCFG_AF_D | PINCFG_PULLUP); // SERCOM4/PAD0 = SDA
  hwpinctrl.PinSetup(PORTNUM_B,  9, PINCFG_AF_D | PINCFG_PULLUP); // SERCOM4/PAD1 = SCL
  i2capp.devnum = 4;

  #define I2C_IRQ_NUM                   62  // SERCOM4_0: PREC
  #define I2C_IRQ_HANDLER   IRQ_Handler_62

  // additional IRQ vectors are required here
  setup_irq(63);
  setup_irq(64);
  setup_irq(65);
}

// This processor has per interrupt flag an interrupt line

extern "C" void IRQ_Handler_63() // SERCOM4_1: AMATCH
{
  i2capp.HandleIrq();
}

extern "C" void IRQ_Handler_64() // SERCOM4_2: DRDY
{
  i2capp.HandleIrq();
}

extern "C" void IRQ_Handler_65() // SERCOM4_3_7: ERROR
{
  i2capp.HandleIrq();
}


#else
  #error "define init_i2c_pins() here"
#endif

#if defined(I2C_IRQ_HANDLER)

extern "C" void I2C_IRQ_HANDLER()
{
  i2capp.HandleIrq();
}

#else
  #error "define the I2C_IRQ_HANDLER!"
#endif

void init_i2c_slave()
{
  init_i2c_pins();

	// Setup the I2C Application

	if (!i2capp.Init(0x50, 0x00))
	{
		TRACE("I2C Slave Application Init FAILED!\r\n");
		return;
	}

	// initialize the data with some content
	i2capp.data[0] = 0x33;
	i2capp.data[1] = 0x44;
	i2capp.data[2] = 0x55;
	i2capp.data[3] = 0x66;
	i2capp.data[4] = 0x77;
	i2capp.data[5] = 0x88;
	i2capp.data[6] = 0x99;
	i2capp.data[7] = 0xAA;
	i2capp.data[8] = 0xBB;
	i2capp.data[9] = 0xCC;
	i2capp.data[10] = 0xDD;

	// Enable The I2C IRQ
  setup_irq(I2C_IRQ_NUM);

  TRACE("I2C Slave Application Initialized.\r\n");
}

// ----------------------------------------------------------------------------
