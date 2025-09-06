
#include "stdint.h"
#include "stdio.h"
#include <unistd.h>  // for usleep()

#include "board_pins.h"
#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"

unsigned hbcounter = 0;

#define HAS_PMU 0

int main()
{
	setbuf(stdout, NULL);  // disable output buffering, useful if this application is controlled externally,
	                       // and the output messages appear immediately

	clockcnt_init();
	board_pins_init();


	TRACE("VIHAL Linux GPIO Test\n");
	TRACE("Board: %s\n", BOARD_NAME);
	TRACE("Clock Cnt Speed = %u\n", SystemCoreClock);

	clockcnt_t t;

#if HAS_PMU
	clockcnt_pmu_init();  // requires a kernel module !

	clockcnt_t t0, t1;

	uint64_t a0 = clockcnt_pmu_get();
	t0 = CLOCKCNT;
	uint64_t a1 = clockcnt_pmu_get();
	t1 = CLOCKCNT;
	uint64_t a2 = clockcnt_pmu_get();

	TRACE("CLOCKCNT readout clocks: %u, %u\n", uint32_t(a1-a0), uint32_t(a2-a1));
	TRACE("  t0=%u, t1=%u\n", t0, t1);
#endif

#if 0

	TRACE("Testing fast GPIO toggling...\r\n");
	TGpioPin * pin = &pin_led[0];
	pin->Set1();
	pin->Set0();
	pin->Set1();
	pin->Set0();
	pin->Set1();
	pin->Set0();
	pin->Set1();
	pin->Set0();

#endif

	while (true)
	{
		t = CLOCKCNT;

		++hbcounter;

    for (unsigned n = 0; n < pin_led_count; ++n)
    {
      pin_led[n].SetTo((hbcounter >> n) & 1);
    }

		printf("%u. hb, clockcnt=%u\n", hbcounter, uint32_t(t));

		usleep(500000);
	}

	return 0;
}
