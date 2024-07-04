
#include "stdint.h"
#include "stdio.h"
#include <unistd.h>  // for usleep()

#include "board_pins.h"
#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"

unsigned hbcounter = 0;

int main()
{
	setbuf(stdout, NULL);

	clockcnt_init();
	board_pins_init();

	TRACE("VIHAL Linux TFT Display Test\n");
	TRACE("Board: %s\n", BOARD_NAME);
	TRACE("Clock Cnt Speed = %u\n", SystemCoreClock);

  TRACE("Initializing board display...\r\n");

  board_display_init();

	disp.FillScreen(0x8000); // fill the screen with blue

  disp.color = 0xffff;
  disp.SetCursor(0, 32);
  disp.printf("1234567890123456789012345678");


  TRACE("Starting main loop.\r\n");

	while (true)
	{
		++hbcounter;

    for (unsigned n = 0; n < pin_led_count; ++n)
    {
      pin_led[n].SetTo((hbcounter >> n) & 1);
    }

    disp.color = 0xffff;
    disp.SetCursor(0, 0);
    disp.printf("%u. hb, clk=%10u", hbcounter, CLOCKCNT);

		usleep(500000);
	}

	return 0;
}
