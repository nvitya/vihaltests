
#include "test_dac.h"
#include "hwpins.h"
#include "board_pins.h"
#include "traces.h"
#include "clockcnt.h"

#include "board_pins.h"

unsigned g_last_clock = 0;
unsigned g_dac_cnt = 0;

uint16_t dac_pattern[4096];

void test_dac_init()
{
	__NOP();  // for breakpoint

#if 1
  dac.SetTo(0);
  dac.SetTo(0x500);
  dac.SetTo(0xF00);
  dac.SetTo(0x100);
#endif

  //dac.SetFrequency(500000 / 2);  // the slew rate of the outputs are too slow for 1 MHz output

	for (unsigned n = 0; n < 4096; ++n)
	{
		//dac_pattern[n] = (n & 0x3F) + ((15*(n & 1)) << 8);
		dac_pattern[n] = n; // sawtooth
	}

  dac.RepeatPattern(&dac_pattern[0], 4096);

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

