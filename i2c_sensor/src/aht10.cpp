/*
 * aht10.cpp
 *
 *  Created on: Feb 12, 2022
 *      Author: vitya
 */

#include <aht10.h>
#include "clockcnt.h"

bool TAht10::Init(THwI2c * ai2c, uint8_t aaddr)
{
  initialized = false;

  pi2c = ai2c;
  addr = aaddr;

  state = 0;
  measurement_count = 0;
  measure_iv_clocks = SystemCoreClock * 3;  // 3s
  command_delay_clocks = 100 * (SystemCoreClock / 1000);
  last_measure = CLOCKCNT;

  initialized = true;
  return true;
}

void TAht10::Run()
{
  if (0 == state) // wait a little for the initialization
  {
    if (CLOCKCNT - last_measure > SystemCoreClock / 10) // wait 100ms
    {
      state = 100; // start initialization
    }
  }
  else if (1 == state) // start measurement
  {
    buf[0] = 0xAC; // write start measurement register
    buf[1] = 0x33; // start measurement control
    buf[2] = 0x00; // nop

    pi2c->StartWriteData(addr, 0, &buf[0], 3);
    state = 2;
  }
  else if (2 == state) // wait for i2c transaction end
  {
    if (!pi2c->Finished())
    {
      return;
    }

    if (pi2c->error)
    {
      state = 10;
      return;
    }

    // delay 10 ms after the command
    last_measure = CLOCKCNT;
    state = 3;
  }
  else if (3 == state) // 10 ms delay
  {
    if (CLOCKCNT - last_measure < command_delay_clocks)
    {
      return;
    }

    // read status and data
    pi2c->StartReadData(addr, 0, &buf[0], 6);
    state = 4;
  }
  else if (4 == state)
  {
    if (!pi2c->Finished())
    {
      return;
    }

    if (pi2c->error)
    {
      state = 10;
      return;
    }

    // check status
    if (buf[0] & 0x80)  // bit7: 1=busy
    {
      // repeat the read out when it was busy
      state = 3;
      return;
    }

    // measurement ok, process the data
    ic_status = buf[0];
    rh_raw = (buf[1] << 12) | (buf[2] << 4) | ((buf[3] >> 4) & 0x0F);  // 20 bit
    t_raw  = ((buf[3] & 0x0F) << 16) | (buf[4] << 8) | buf[5];

    // convert to integers
    rh_percent_x100 = (10000 * (rh_raw >> 4)) >> 16;
    t_deg_x100 = ((20000 * (t_raw >> 4)) >> 16) - 5000;

    ++measurement_count;

    state = 10; // delay for the next measurement
  }

  // measure delay

  else if (10 == state) // start measurement delay
  {
    last_measure = CLOCKCNT;
    state = 11;
  }
  else if (11 == state)
  {
    if (CLOCKCNT - last_measure >= measure_iv_clocks)
    {
      state = 1; // repeat the measurement
    }
  }

  // initialization

  else if (100 == state) // start initialziation
  {
    buf[0] = 0xBA; // soft reset register
    pi2c->StartWriteData(addr, 0, &buf[0], 1);
    state = 101;
    last_measure = CLOCKCNT;
  }
  else if (101 == state) // wait reset complete
  {
    if (!pi2c->Finished())
    {
      return;
    }

    if (pi2c->error)
    {
      state = 100; // repeat write
      return;
    }

    last_measure = CLOCKCNT;
    state = 102; // delay after the reset
  }
  else if (102 == state)  // wait for mode set
  {
    if (CLOCKCNT - last_measure < 20 * (SystemCoreClock / 1000))
    {
      return;
    }

    // set normal mode
    buf[0] = 0xE1; // initialization register
    buf[1] = 0x08; // normal mode, calibration on
    buf[2] = 0x00; // nop

    pi2c->StartWriteData(addr, 0, &buf[0], 3);
    state = 103;
  }
  else if (103 == state) // wait for mode complete
  {
    if (!pi2c->Finished())
    {
      return;
    }

    if (pi2c->error)
    {
      state = 102; // repeat write
      return;
    }

    last_measure = CLOCKCNT;
    state = 110; // delay after init
  }
  else if (110 == state)
  {
    if (CLOCKCNT - last_measure >= 20 * (SystemCoreClock / 1000))
    {
      state = 1; // start the normal measurement cycle
    }
  }

}
