/*
 * bmp280.h
 *
 *  Created on: Feb 9, 2022
 *      Author: vitya
 */

#ifndef SRC_BMP280_H_
#define SRC_BMP280_H_

#include "stdint.h"
#include "hwi2c.h"
#include "i2cmanager.h"

typedef struct
{
  uint16_t   T1; // 0x88/0x89
  int16_t    T2;
  int16_t    T3;

  uint16_t   P1;
  int16_t    P2;
  int16_t    P3;
  int16_t    P4;
  int16_t    P5;
  int16_t    P6;
  int16_t    P7;
  int16_t    P8;
  int16_t    P9;

  uint16_t   res;
//
} bmp280_dig_t;

class TBmp280
{
public:
  bool      initialized = false;
  uint8_t   addr = 0x77;
  uint8_t   id = 0;

  TI2cManager *  pi2cmgr = nullptr;

  uint32_t  last_measure = 0;
  uint32_t  measure_iv_clocks = 0;

  int32_t   t_celsius_x100 = 0;  // temperature in 0.01 Celsius degrees
  uint32_t  p_pascal_Q24 = 0;
  uint32_t  p_pascal     = 0;

  int32_t   p_raw = 0;
  int32_t   t_raw = 0;

  uint8_t   ic_status = 0;
  uint8_t   ic_control = 0;
  uint8_t   ic_config = 0;

  int32_t   t_fine = 0;

  bmp280_dig_t  dig; // use the "dig" like in the calculation formulas

  uint32_t  measure_count = 0; // incremented after every successful measurement
  uint32_t  prev_measure_count = 0; // variable provided for the application

  bool      Init(TI2cManager * ai2cmgr, uint8_t aaddr);
  void      Run();

  int32_t   CalculateTemp(int32_t adc_T);
  uint32_t  CalculatePressure(int32_t adc_P);

protected:
  int       state = 0;

  TI2cTransaction tra;

  uint8_t   buf[16];
};

#endif /* SRC_BMP280_H_ */
