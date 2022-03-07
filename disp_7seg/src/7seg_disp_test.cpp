// file:     tft_disp_test.cpp
// brief:    TFT LCD Display Test
// created:  2022-03-04
// authors:  nvitya

#include "platform.h"

#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"
#include "board_pins.h"

unsigned g_disp_counter = 0;
unsigned g_prev_scannum = 0;
unsigned g_prev_keys = 0;
unsigned g_prev_slowcnt = 0;
unsigned g_slowcnt = 0;

void disp_test_init()
{
  // nothing
}

void disp_test_run()
{
  ++g_disp_counter;

  unsigned t = CLOCKCNT;
  if (t - g_prev_slowcnt > SystemCoreClock / 20)
  {
    ++g_slowcnt;
    g_prev_slowcnt = t;
  }

  disp.Run();

  #if 0 == DISP_SEL

    if (g_prev_keys != disp.keys)
    {
      TRACE("Keys changed to %02X\r\n", disp.keys);
      g_prev_keys = disp.keys;
    }

    disp.leds = (g_slowcnt & 0xFF); // some binary counter
    disp.dots = disp.keys; // mirror back the keys on the leds

    if (g_prev_scannum != disp.controller.scancounter)
    {
      disp.DisplayDecNum(g_disp_counter);
      g_prev_scannum = disp.controller.scancounter;
    }

  #elif 1 == DISP_SEL

    disp.colon_on = ((g_slowcnt & (1 << 3)) != 0);

    if (g_prev_scannum != disp.controller.scancounter)
    {
      disp.DisplayDecNum(g_slowcnt);
      g_prev_scannum = disp.controller.scancounter;
    }

  #elif 2 == DISP_SEL

    //disp.dots = disp.keys; // mirror back the keys on the leds

    if (g_prev_scannum != disp.scancounter)
    {
      disp.DisplayHexNum(g_disp_counter);
      g_prev_scannum = disp.scancounter;
    }
  #elif 3 == DISP_SEL

    if (g_prev_scannum != disp.scancounter)
    {
      disp.DisplayDecNum(g_slowcnt);
      g_prev_scannum = disp.scancounter;
    }
  #else
    #error "implement display run"
  #endif
}

