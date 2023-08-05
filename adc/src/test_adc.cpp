/*
 *  file:     test_adc.cpp
 *  brief:    ADC Test
 *  date:     2021-11-13
 *  authors:  nvitya
 *
 *  description:
 *    two ADC channels are continously measured and printed to the UART console
 *    I was using an arduino analogue joystick board for the test.
*/

#include "stdint.h"
#include "stdlib.h"
#include "platform.h"
#include "hwpins.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "board_pins.h"
#include "traces.h"

#include "hwadc.h"

THwAdc adc;

#define ADC_REC_LEN  1024

uint16_t adc_rec_buffer[ADC_REC_LEN];

#define ADC_ANALYZE_LEN  256

uint16_t adc_value_cnt[ADC_ANALYZE_LEN];

uint8_t adc_ch_x;
uint8_t adc_ch_y;
uint8_t adc_num;

uint8_t adc_shift = 6; // keep only the highest 10 bits

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

#elif defined(BOARD_WEMOS_C3MINI)

void adc_setup()
{
  hwpinctrl.PinSetup(0, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ch0
  hwpinctrl.PinSetup(0, 1, PINCFG_INPUT | PINCFG_ANALOGUE); // ch1

  adc_num = 1;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_MIN_F103) || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
    || defined(BOARD_MIBO64_STM32F405)

void adc_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ch0
  hwpinctrl.PinSetup(PORTNUM_A, 1, PINCFG_INPUT | PINCFG_ANALOGUE); // ch1

  adc_num = 1;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#elif defined(BOARD_MIBO48_STM32F303) || defined(BOARD_MIBO48_STM32G473)

void adc_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ch1
  hwpinctrl.PinSetup(PORTNUM_A, 1, PINCFG_INPUT | PINCFG_ANALOGUE); // ch2

  adc_num = 1;
  adc_ch_x = 1;
  adc_ch_y = 2;
}

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743) || defined(BOARD_NUCLEO_H723)

void adc_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A, 3, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC123_in3
  hwpinctrl.PinSetup(PORTNUM_C, 0, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC123_in10

  adc_num = 1;
  adc_ch_x = 3;
  adc_ch_y = 10;
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE) || defined(BOARD_MIBO64_ATSAM4S)

void adc_setup()
{
  // The ATSAM does not require pin setup, the pins switched automatically to analogue mode on channel enabling

  // CH0, CH1 Pins:
  // Atsam-4S: PA17, PA18
  // Arduino DUE: PA2(AD7), PA3(AD6)
  // ATSAM-E70: PD30, PA21

  adc_num = 0;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#elif defined(BOARD_XPLAINED_SAME70)

void adc_setup()
{
  // The ATSAM does not require pin setup, the pins switched automatically to analogue mode on channel enabling

  adc_num = 0;
  adc_ch_x = 0;  // PD30 (AD2): AFE0_AD0
  adc_ch_y = 8;  // PA19 (AD3): AFE0_AD8
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void adc_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A, 2, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC0_AIN[0]
  hwpinctrl.PinSetup(PORTNUM_A, 3, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC0_AIN[1]

  adc_num = 0;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void adc_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A, 2, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC0_AIN[0]
  hwpinctrl.PinSetup(PORTNUM_A, 3, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC0_AIN[1]

  adc_num = 0;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

// RP

#elif defined(BOARD_RPI_PICO)

void adc_setup()
{
  hwpinctrl.PinSetup(0, 26, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC0
  hwpinctrl.PinSetup(0, 27, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC1
  hwpinctrl.PinSetup(0, 28, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC2
  hwpinctrl.PinSetup(0, 29, PINCFG_INPUT | PINCFG_ANALOGUE); // ADC3 (internal, VCC measurement)

  adc_num = 0;
  adc_ch_x = 0;
  adc_ch_y = 1;
}

#else
  #error "ADC board specific setup is missing"
#endif

//------------------------------------------------------------------------------------------------------------

void analyze_record()
{
  int i;
  for (i = 0; i < ADC_ANALYZE_LEN; ++i)
  {
    adc_value_cnt[i] = 0;
  }

  // search min-max
  uint16_t minval = 0xFFFF;
  uint16_t maxval = 0;
  for (i = 0; i < ADC_REC_LEN; ++i)
  {
    uint16_t v = (adc_rec_buffer[i] << HWADC_DATA_LSHIFT) >> 4; // 12 bit resolution
    if (v < minval)  minval = v;
    if (v > maxval)  maxval = v;
  }

  if (maxval - minval > ADC_ANALYZE_LEN)
  {
    TRACE("Min-Max range is out of analyze buffer: %i\r\n", maxval-minval);
    return;
  }

  // collect counts

  for (i = 0; i < ADC_REC_LEN; ++i)
  {
    uint16_t v = (adc_rec_buffer[i] << HWADC_DATA_LSHIFT) >> 4; // 12 bit resolution
    int idx = v - minval;
    ++adc_value_cnt[idx];
  }

  // displaying data
  uint16_t maxcnt = 0;
  uint16_t maxidx = 0;
  TRACE(" min = %u, max = %u, diff = %u\r\n", minval, maxval, maxval - minval);
  for (i = 0; i < maxval - minval; ++i)
  {
    TRACE("  %4u : %u\r\n", minval + i, adc_value_cnt[i]);
    if (adc_value_cnt[i] > maxcnt)
    {
      maxcnt = adc_value_cnt[i];
      maxidx = i;
    }
  }

  TRACE(" modus = %u\r\n", minval + maxidx);
}

void test_adc_record(uint8_t achnum)
{
  TRACE("ADC record test for channel %i\r\n", achnum);

  unsigned t0, t1;

  int state = 0;

  t0 = CLOCKCNT;
  while (1)
  {
    if (0 == state)
    {
      pin_led[0].Toggle();

      int i;
      for (i = 0; i < ADC_REC_LEN; ++i)  adc_rec_buffer[i] = 0x1111 + i;

      adc.StartRecord((1 << achnum), sizeof(adc_rec_buffer), &adc_rec_buffer[0]);
      t0 = CLOCKCNT;
      state = 1;
    }
    else if (1 == state)
    {
      if (adc.RecordFinished())
      {
        t1 = CLOCKCNT;
        TRACE("Record finished in %u clocks\r\n", t1 - t0);
        analyze_record();
        t0 = CLOCKCNT;
        state = 2; // wait
      }
    }
    else if (2 == state)
    {
      t1 = CLOCKCNT;
      if (t1 - t0 > SystemCoreClock / 1)
      {
        state = 0; // start again
      }
    }

    //idle_task();
  }
}

void adc_test_freerun()
{
  TRACE("*** ADC Test Freerun ***\r\n");

  unsigned t0, t1;
  unsigned hbcounter = 0;

  unsigned dcnt = 0;

  unsigned sampspeed = 20;

  t0 = CLOCKCNT;
  while (1)
  {
    t1 = CLOCKCNT;
    if (t1 - t0 > SystemCoreClock / sampspeed)
    {
      uint16_t advx = (adc.ChValue(adc_ch_x) >> adc_shift);
      uint16_t advy = (adc.ChValue(adc_ch_y) >> adc_shift);

      TRACE("\rx: %5u, y: %5u   ", advx, advy);

      // and finally do the usual led blinking

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((hbcounter >> n) & 1);
      }

      ++hbcounter;
      t0 = t1;
    }
  }
}

void test_adc()
{
  TRACE("Testing ADC\r\n");

  adc_setup();

  adc.Init(adc_num, (1 << adc_ch_x) | (1 << adc_ch_y));
  //adc.Init(adc_num, (1 << adc_ch_x));

  TRACE("ADC speed: %u conversions / s\r\n", adc.act_conv_rate);

  adc_test_freerun();  // does not return

  //test_adc_record(adc_ch_x);
  //test_adc_record(adc_ch_y);

  TRACE("ADC test finished.\r\n");
}

// ----------------------------------------------------------------------------
