/*
 * ledstripe_test.cpp
 *
 *  Created on: Mar 23, 2022
 *      Author: vitya
 */

#include "board_pins.h"
#include "traces.h"
#include "ledstripe.h"

#if LEDSTRIPE_WS2813
  TLedStripe_ws2813  ledstripe;
#elif LEDSTRIPE_APA102
  TLedStripe_apa102  ledstripe;
#else
  #error "unhandled config"
#endif

uint8_t led_data_buf[4096];

unsigned last_update = 0;
unsigned curpos = 0;
unsigned upcount = 1;

void update_leds()
{
  for (unsigned n = 0; n < ledstripe.ledcount; ++n)
  {
    if (n == curpos)
    {
      ledstripe.SetRGB(n, 255, 0, 0);
    }
    else
    {
      ledstripe.SetRGB(n, 0, 0, 0);
    }
  }

  ledstripe.Update();
}

void ledstripe_test_init()
{
  #if LEDSTRIPE_APA102
    ledstripe.intensity = 63; // global intensity setting
  #endif
  ledstripe.Init(&fl_spi, LED_COUNT, &led_data_buf[0], sizeof(led_data_buf));
  curpos = 0;
  update_leds();
}

void ledstripe_test_run()
{
  ledstripe.Run();

  unsigned t = CLOCKCNT;
  if (t - last_update > SystemCoreClock / 40)
  {
    if (upcount)
    {
      if (curpos < ledstripe.ledcount - 1)
      {
        ++curpos;
      }
      else
      {
        upcount = 0;
        curpos = ledstripe.ledcount - 2;
      }
    }
    else
    {
      if (curpos > 0)
      {
        --curpos;
      }
      else
      {
        upcount = 1;
        curpos = 1;
      }
    }

    update_leds();

    last_update = t;
  }
}

