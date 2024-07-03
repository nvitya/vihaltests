
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

	TRACE("VIHAL Linux GPIO Test\n");
	TRACE("Board: %s\n", BOARD_NAME);
	TRACE("Clock Cnt Speed = %u\n", SystemCoreClock);

	while (true)
	{
		++hbcounter;

    for (unsigned n = 0; n < pin_led_count; ++n)
    {
      pin_led[n].SetTo((hbcounter >> n) & 1);
    }

		printf("%u. hb, clockcnt=%u\n", hbcounter, CLOCKCNT);

		usleep(500000);
	}

	return 0;
}
