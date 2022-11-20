
#include "test_dac.h"
#include "hwpins.h"
#include "board_pins.h"
#include "traces.h"
#include "clockcnt.h"

#include "board_pins.h"

unsigned g_last_clock = 0;
unsigned g_dac_cnt = 0;

uint16_t dac_pattern[1024];

void test_dac_init()
{
  dac_pattern[0] = 0x0010;
  dac_pattern[1] = 0x0F20;
  dac_pattern[2] = 0x0030;
  dac_pattern[3] = 0x0F40;

  dac.SetTo(0);
  dac.SetTo(0x500);
  dac.SetTo(0xF00);
  dac.SetTo(0x100);

  dac.RepeatPattern(&dac_pattern[0], 4);

  g_last_clock = CLOCKCNT;
}


void test_dac_run()
{
  return;

  unsigned t = CLOCKCNT;
  if (t - g_last_clock > SystemCoreClock / (1000000 * 2))
  {
    ++g_dac_cnt;
    if (g_dac_cnt & 1)
    {
      dac.SetTo(0x0400);
    }
    else
    {
      dac.SetTo(0x0000);
    }

    g_last_clock = t;
  }

}

