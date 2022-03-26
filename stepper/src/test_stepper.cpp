/*
 *  file:     test_stepper.cpp
 *  brief:    Stepper Motor Test
 *  date:     2022-03-26
 *  authors:  nvitya
*/

#include "stdint.h"
#include "stdlib.h"
#include "hwpins.h"
#include "clockcnt.h"
#include "board_pins.h"
#include "traces.h"

int   range = 2048;  // exactly one turn
bool  up = true;
int   state = 0;


void test_stepper_init()
{
  if (!stepper.Init())
  {
    TRACE("Stepper motor initialization failed!\r\n  (probably pin assignments missing)\r\n");
    return;
  }

  TRACE("Stepper Motor initialized.\r\n");
}

void test_stepper_run()
{
  stepper.Run();

  if (0 == state)
  {
    if (up)
    {
      stepper.position += range;
    }
    else
    {
      stepper.position -= range;
    }
    up = !up;
    state = 1;
  }
  else if (1 == state)
  {
    if (stepper.position == stepper.actual_pos)
    {
      state = 0;
    }
  }
}


// ----------------------------------------------------------------------------
