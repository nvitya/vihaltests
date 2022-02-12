/*
 * bmp280.cpp
 *
 *  Created on: Feb 9, 2022
 *      Author: vitya
 */

#include <bmp280.h>
#include "traces.h"

bool TBmp280::Init(TI2cManager * ai2cmgr, uint8_t aaddr)
{
  pi2cmgr = ai2cmgr;
  addr = aaddr;

  initialized = false;

  TRACE("Initializing BMP280\r\n");

  // read id:
  pi2cmgr->AddRead(&tra, addr, 0xD0 | I2CEX_1, &id, 1);
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error reading BMP-280 ID!\r\n");
    return false;
  }

  TRACE(" ID = %02X\r\n", id);

  // read status and config

  pi2cmgr->AddRead(&tra, addr, 0xF3 | I2CEX_1, &buf[0], 3);
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error reading BMP-280 status!\r\n");
    return false;
  }

  TRACE(" F3 STATUS = %02X\r\n", buf[0]);
  TRACE(" F4 CTRL   = %02X\r\n", buf[1]);
  TRACE(" F5 CONFIG = %02X\r\n", buf[2]);

  // reading calibration / "dig" values
  pi2cmgr->AddRead(&tra, addr, 0x88 | I2CEX_1, &dig, sizeof(dig));
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error reading BMP-280 calibration data!\r\n");
    return false;
  }

  // stop first
  buf[0] = 0;
  pi2cmgr->AddWrite(&tra, addr, 0xF4 | I2CEX_1, &buf[0], 1);
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error writing BMP-280 CTRL!\r\n");
    return false;
  }

  // set F5 CONFIG:
  buf[0] = (0
    | (0  <<  0)  // spi3w_en:
    | (8  <<  2)  // filter(3): time constant for the IIR filter
    | (2  <<  5)  // t_sb(3): 2 = 125us, 3 = 250 us, 4 = 500 us, 5 = 1000 us
  );
  pi2cmgr->AddWrite(&tra, addr, 0xF5 | I2CEX_1, &buf[0], 1);
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error writing BMP-280 CONFIG!\r\n");
    return false;
  }

  // start, set F4 CTRL:
  buf[0] = (0
    | (3  <<  0)  // mode(2): 3 = normal mode
    | (4  <<  2)  // osrs_p(3): 3 = 4x oversampling (pressure), 4 = 8x oversampling
    | (4  <<  5)  // osrs_t(3): 3 = 4x oversampling (temp), 4 = 8x oversampling
  );
  pi2cmgr->AddWrite(&tra, addr, 0xF4 | I2CEX_1, &buf[0], 1);
  pi2cmgr->WaitFinish(&tra);
  if (tra.errorcode)
  {
    TRACE("Error writing BMP-280 config!\r\n");
    return false;
  }

  state = 0;
  measure_count = 0;
  measure_iv_clocks = SystemCoreClock;  // 1s

  initialized = true;

  return true;
}

// Original BOSH functions with different names only:

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t TBmp280::CalculateTemp(int32_t adc_T)
{
  int32_t var1, var2, T;
  var1 = ((((adc_T >> 3) - ((int32_t)dig.T1 << 1))) * ((int32_t)dig.T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((int32_t)dig.T1)) * ((adc_T >> 4) - ((int32_t)dig.T1))) >> 12) * ((int32_t)dig.T3)) >> 14;
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t TBmp280::CalculatePressure(int32_t adc_P)
{
  int64_t var1, var2, p;
  var1 = ((int64_t)t_fine) - 128000;

  var2 = var1 * var1 * (int64_t)dig.P6;
  var2 = var2 + ((var1*(int64_t)dig.P5) << 17);
  var2 = var2 + (((int64_t)dig.P4) << 35);
  var1 = ((var1 * var1 * (int64_t)dig.P3) >> 8) + ((var1 * (int64_t)dig.P2) << 12);
  var1 = (((((int64_t)1) << 47)+var1))*((int64_t)dig.P1) >> 33;
  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)dig.P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)dig.P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig.P7) << 4);
  return (uint32_t)p;
}

void TBmp280::Run()  // non-blocking I2C state machine
{
  if (!pi2cmgr)
  {
    return;
  }

  pi2cmgr->Run();

  if (!initialized)
  {
    return;
  }

  if (0 == state)  // start the measurement
  {
    // burst read results always consistent reading:
    pi2cmgr->AddRead(&tra, addr, 0xF3 | I2CEX_1, &buf[3], 10);  // index corresponds to the register address
    state = 1;
  }
  else if (1 == state) // wait for the I2C transaction finish
  {
    if (tra.completed)
    {
      // process the results

      ic_status = buf[3];
      ic_control = buf[4];
      ic_config = buf[5];

      p_raw = (buf[9] >> 4) | (buf[8] << 4) | (buf[7] << 12);
      t_raw = (buf[12] >> 4) | (buf[11] << 4) | (buf[10] << 12);

      t_celsius_x100 = CalculateTemp(t_raw);
      p_pascal_Q24 = CalculatePressure(p_raw);
      p_pascal = (p_pascal_Q24 >> 8);

      last_measure = CLOCKCNT;

      ++measure_count;

      state = 10;
    }
  }
  else if (10 == state) // delay
  {
    if (CLOCKCNT - last_measure >= SystemCoreClock)
    {
      state = 0;
    }
  }

}
