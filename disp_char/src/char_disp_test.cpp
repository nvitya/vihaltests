// file:     char_disp_test.cpp
// brief:    Character LCD Display Test
// created:  2022-03-09
// authors:  nvitya

#include "platform.h"

#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"
#include "mscounter.h" // has much wider range than the clockcnt
#include "board_pins.h"

#include "char_disp_test.h"

unsigned g_slowcnt = 0;
unsigned g_lastslowtime = 0;

void disp_test_init()
{
  disp.Clear();
  disp.SetPos(0, 0);
  disp.printf("Hello World!\n");
  disp.printf("Second line...");
  // force update:
  while (disp.screenchanged)
  {
    disp.Run();
  }

  g_lastslowtime = CLOCKCNT;
}

void disp_test_run()
{
  unsigned t0 = CLOCKCNT;
  if (t0 - g_lastslowtime > SystemCoreClock / 10)
  {
    ++g_slowcnt;
    disp.SetPos(0, 1);
    disp.printf("cnt = %6u   ", g_slowcnt);
    g_lastslowtime = t0;
  }

  disp.Run();
}

