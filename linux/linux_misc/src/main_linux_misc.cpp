
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
	setbuf(stdout, NULL);  // disable output buffering, useful if this application is controlled externally,
	                       // and the output messages appear immediately

	clockcnt_init();
	board_pins_init();

	printf("----------------------------------------\n");
	printf("VIHAL Misc. Linux Test\n");
	printf("Check the UART output!\n");

	TRACE("VIHAL Linux UART Test\r\n");
	TRACE("Board: %s\r\n", BOARD_NAME);
	TRACE("Clock Cnt Speed = %u\r\n", SystemCoreClock);

	clockcnt_t t;

	printf("Starting main cycle\n");
	TRACE("Starting main cycle\r\n");
	while (true)
	{
		t = CLOCKCNT;

		++hbcounter;

    for (unsigned n = 0; n < pin_led_count; ++n)
    {
      pin_led[n].SetTo((hbcounter >> n) & 1);
    }

		//printf("%u. hb, clockcnt=%u\n", hbcounter, uint32_t(t));
		TRACE("%u. hb, clockcnt=%u\r\n", hbcounter, uint32_t(t));

		usleep(500000);
	}

	return 0;
}
