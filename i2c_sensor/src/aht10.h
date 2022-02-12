/*
 * aht10.h
 *
 *  Created on: Feb 12, 2022
 *      Author: vitya
 */

#ifndef SRC_AHT10_H_
#define SRC_AHT10_H_

#include "stdint.h"
#include "hwi2c.h"
#include "i2cmanager.h"

class TAht10
{
public:
  bool      initialized = false;
  uint8_t   addr = 0x77;
  TI2cManager * pi2cmgr = nullptr;

  int       t_deg_x100 = 0;
  uint32_t  rh_percent_x100 = 0;

  uint32_t  rh_raw = 0;  // 20 bit scaling: 0 = 0%,  0x100000 = 100 %
  uint32_t  t_raw = 0;   // 20 bit scaling: 0 = -50 C, 0x100000 = 150 C

  uint8_t   ic_status = 0;

  uint32_t  measure_count = 0; // incremented after every successful measurement
  uint32_t  prev_measure_count = 0; // variable provided for the application

  bool      Init(TI2cManager * ai2cmgr, uint8_t aaddr);
  void      Run();

protected:
  int       state = 0;

  uint32_t  last_measure = 0;
  uint32_t  measure_iv_clocks = 0;
  uint32_t  command_delay_clocks = 0;

  uint8_t   buf[8];  // internaly used

  TI2cTransaction tra;
};

#endif /* SRC_AHT10_H_ */
