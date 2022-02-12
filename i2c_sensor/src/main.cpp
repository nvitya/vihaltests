// file:     main.cpp (i2c_sensor)
// brief:    VIHAL I2C SensorTest
// created:  2022-02-09
// authors:  nvitya

#include "platform.h"
#include "cppinit.h"
#include "clockcnt.h"

#include "hwclk.h"
#include "hwpins.h"
#include "hwuart.h"
#include "traces.h"

#include "board_pins.h"
#include "bmp280.h"
#include "aht10.h"
#include "i2cmanager.h"

volatile unsigned hbcounter = 0;

TBmp280  bmp280;
TAht10   aht10;

TI2cManager  i2cmgr;

void sensor_init()
{
  bmp280.Init(&i2cmgr, 0x76);

  aht10.Init(&i2cmgr, 0x38);
}

void sensor_run()  // runs from idle, not from heartbeat !
{
  i2cmgr.Run();

  bmp280.Run();
  aht10.Run();

  if (aht10.measure_count != aht10.prev_measure_count)
  {
    TRACE("AHT10 ST=%02X, T = %u, RH = %u\r\n", aht10.ic_status, aht10.t_deg_x100, aht10.rh_percent_x100);

    aht10.prev_measure_count = aht10.measure_count;
  }

  if (bmp280.measure_count != bmp280.prev_measure_count)
  {
    TRACE("BMP280 ST=%02X, CTRL=%02X, CFG=%02X", bmp280.ic_status, bmp280.ic_control, bmp280.ic_config);
    TRACE(",  P_RAW=%i, T_RAW=%i", bmp280.p_raw, bmp280.t_raw);
    TRACE(",  T=%i, P=%i", bmp280.t_celsius_x100, bmp280.p_pascal);
    TRACE("\r\n");

    bmp280.prev_measure_count = bmp280.measure_count;
  }
}

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();

  // run the C/C++ initialization (variable initializations, constructors)
  cppinit();

#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else
  if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
  {
    while (1)
    {
      // error
    }
  }
#endif

	mcu_enable_fpu();    // enable coprocessor if present
	mcu_enable_icache(); // enable instruction cache if present

	clockcnt_init();

	// go on with the hardware initializations
	board_pins_init();

	i2cmgr.Init(&i2c);

	TRACE("\r\n--------------------------------------\r\n");
	TRACE("VIHAL I2C Sensor Test\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

	mcu_enable_interrupts();

	sensor_init();

	TRACE("Starting main cycle...\r\n");

	unsigned hbclocks = SystemCoreClock / 2;

	unsigned t0, t1;

	t0 = CLOCKCNT;

	// Infinite loop
	while (1)
	{
		t1 = CLOCKCNT;

		sensor_run();

		if (t1-t0 > hbclocks)
		{
			++hbcounter;

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

			t0 = t1;
		}
	}
}

// ----------------------------------------------------------------------------
