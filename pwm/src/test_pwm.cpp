/*
 *  file:     test_pwm.cpp
 *  brief:    PWM Test
 *  date:     2021-11-14
 *  authors:  nvitya
 *
 *  description:
 *    controlling a small modell servo motor with the PWM unit
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
#include "board_pins.h"

#include "hwpwm.h"

THwPwmChannel      pwm[2];

#if 0  // to use elif everywhere

//-------------------------------------------------------------------------------
// Risc-V (RV32I)
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// ARM Cortex-M
//-------------------------------------------------------------------------------

// STM32

#elif defined(BOARD_MIN_F103) || defined(BOARD_MIN_F401) || defined(BOARD_MIN_F411) \
    || defined(BOARD_MIBO48_STM32F303) \
    || defined(BOARD_MIBO64_STM32F405) \
    || defined(BOARD_MIBO48_STM32G473)

void pwm_setup()
{
  // PWMs using TIM2
  hwpinctrl.PinSetup(PORTNUM_A,  0,  PINCFG_OUTPUT | PINCFG_AF_1);  // TIM2_CH1
  hwpinctrl.PinSetup(PORTNUM_A,  1,  PINCFG_OUTPUT | PINCFG_AF_1);  // TIM2_CH2

  pwm[0].Init(2, 1, 0);
  pwm[1].Init(2, 2, 0);
}

#elif 0 //defined(BOARD_MIBO48_STM32F303)

#elif defined(BOARD_NUCLEO_F446) || defined(BOARD_NUCLEO_F746) || defined(BOARD_NUCLEO_H743) || defined(BOARD_NUCLEO_H723)

void pwm_setup()
{
  // PWMs using TIM2
  hwpinctrl.PinSetup(PORTNUM_B, 11,  PINCFG_OUTPUT | PINCFG_AF_1);  // TIM2_CH4
  hwpinctrl.PinSetup(PORTNUM_B, 10,  PINCFG_OUTPUT | PINCFG_AF_1);  // TIM2_CH3

  pwm[0].Init(2, 4, 0);
  pwm[1].Init(2, 3, 0);
}

// ATSAM

#elif defined(BOARD_ARDUINO_DUE)

void pwm_setup()
{
  hwpinctrl.PinSetup(PORTNUM_B, 12,  PINCFG_OUTPUT | PINCFG_AF_B);  // D20: PWMH0
  hwpinctrl.PinSetup(PORTNUM_B, 13,  PINCFG_OUTPUT | PINCFG_AF_B);  // D21: PWMH1

  pwm[0].inverted = true;
  pwm[0].Init(0, 0);
  pwm[1].inverted = true;
  pwm[1].Init(0, 1);
}

#elif defined(BOARD_XPLAINED_SAME70)

void pwm_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A,  2,  PINCFG_OUTPUT | PINCFG_AF_A);  // arduino D7: PWM0_H1
  hwpinctrl.PinSetup(PORTNUM_C, 19,  PINCFG_OUTPUT | PINCFG_AF_B);  // arduino D6: PWM0_H2

  pwm[0].inverted = true;
  pwm[0].Init(0, 1);
  pwm[1].inverted = true;
  pwm[1].Init(0, 2);
}

#elif defined(BOARD_MIBO64_ATSAME5X)

void pwm_setup()
{
  hwpinctrl.PinSetup(PORTNUM_A,  8,  PINCFG_OUTPUT | PINCFG_AF_F);  // TCC0-WO[0]
  hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_OUTPUT | PINCFG_AF_F);  // TCC0-WO[1]

  //pwm[0].inverted = true;
  pwm[0].Init(0, 0);
  //pwm[1].inverted = true;
  pwm[1].Init(0, 1);
}

// RP

#elif defined(BOARD_RPI_PICO)

void pwm_setup()
{
  hwpinctrl.PinSetup(0, 14,  PINCFG_OUTPUT | PINCFG_AF_4);  // PWM7_A
  hwpinctrl.PinSetup(0, 15,  PINCFG_OUTPUT | PINCFG_AF_4);  // PWM7_B

  //pwm[0].inverted = true;
  pwm[0].Init(0, 7, 0);
  //pwm[1].inverted = true;
  pwm[1].Init(0, 7, 1);
}

#else
  #error "Board specific PWM setup is missing!"
#endif

void pwm_test_highfreq()
{
  // on some mcu families (including STM32) the channels on the same timer will have the same base frequency
  // for the proper operation we have to set the same frequency for them
  // set 20 kHz base frequency

  unsigned n;
  for (n = 0; n < 2; ++n)
  {
    pwm[n].SetFrequency(200000);
    pwm[n].SetOnClocks(pwm[n].periodclocks >> 1);
    pwm[n].Enable();
  }
}

const uint16_t  servo_ctrl_seq[4] = { 0x8000, 0xFFFF, 0x8000, 0x0000 };  // 0xFFFF = 100%, 0x0000 = 0%

unsigned servo_base;
unsigned servo_range;

void pwm_servo_set(unsigned pwmnum, uint16_t aratio16) // 0 = 0%, 0xFFFF = 100 %
{
  unsigned ctrl = ((aratio16 * servo_range) >> 16);
  pwm[pwmnum].SetOnClocks(servo_base + ctrl);
}

void pwm_test_modellservo()
{
  TRACE("Modell servo test, using PWM1...\r\n");

/* modell servo control:
 *
 *   PWM period:        20 ms (50 Hz)
 *   minimum position:  1 ms high, 19 ms low
 *   maximum position:  2 ms high, 18 ms low
 */

  pwm[0].SetFrequency(50); // 50 hz base cycle
  pwm[1].SetFrequency(50);

  servo_base = pwm[0].periodclocks / 20;  // 1 ms is the minimum
  unsigned fullclocks = pwm[0].periodclocks / 10;  // 2 ms is the full 180 degrees
  servo_range = fullclocks - servo_base;

  TRACE("Control resolution: %u\r\n", servo_range);

  pwm_servo_set(0, servo_ctrl_seq[0]);
  pwm_servo_set(1, servo_ctrl_seq[1]);

  pwm[0].Enable();
  pwm[1].Enable();

  // main cycle

  unsigned slowcnt = 0;
  unsigned slowclocks = SystemCoreClock;
  unsigned t0, t1;
  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    if (t1-t0 > slowclocks)
    {
      ++slowcnt;

      pwm_servo_set(0, servo_ctrl_seq[slowcnt & 3]);
      pwm_servo_set(1, servo_ctrl_seq[(slowcnt + 1) & 3]);

      for (unsigned n = 0; n < pin_led_count; ++n)
      {
        pin_led[n].SetTo((slowcnt >> n) & 1);
      }

      board_show_hexnum(slowcnt);

      t0 = t1;
    }
  }

}

void test_pwm()
{
  TRACE("Testing PWM\r\n");

  pwm_setup();

  //pwm_test_highfreq();
  pwm_test_modellservo();

  TRACE("PWM test finished.\r\n");
}

// ----------------------------------------------------------------------------
