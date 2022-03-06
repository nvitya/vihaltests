// file:     tft_disp_test.cpp
// brief:    TFT LCD Display Test
// created:  2022-03-04
// authors:  nvitya

#include "platform.h"

#include "hwpins.h"
#include "traces.h"
#include "clockcnt.h"
#include "mscounter.h" // has much wider range than the clockcnt
#include "board_pins.h"

#include "gfxbase.h"
#include "gfxtextscreen.h"

#include "mono_disp_test.h"

#define TXTSCR_MAX_WIDTH  80
#define TXTSCR_MAX_HEIGHT 40

uint8_t txtscr_charbuf[TXTSCR_MAX_WIDTH*TXTSCR_MAX_HEIGHT];
uint8_t txtscr_changebuf[sizeof(txtscr_charbuf) / 8];

TGfxTextScreen   textscreen;

TMonoDisplayTest  displaytest;

#include "font_FreeSerifBold9pt7b.h"
TGfxFont font_serif(&FreeSerifBold9pt7b);

//#include "font_FreeMonoBold9pt7b.h"
//TGfxFont font_serif(&FreeMonoBold9pt7b);

void disp_test_run()
{
  displaytest.Run();
}

void disp_test_init()
{
	TRACE("Display Test Init\r\n");

	displaytest.Init();

  if (!disp.initialized)
  {
    TRACE("Display init: ERROR!\r\n");
  }
  else
  {
    TRACE("Display init: OK.\r\n");
  }
}

void TMonoDisplayTest::Init()
{
  testid = START_TEST;
  InitTest();

  test_begin_ms = mscounter();
}

void TMonoDisplayTest::InitTest()
{
  disp.FillScreen(0);

  if (0 == testid)
  {
    test0_init();
  }
  else if (1 == testid)
  {
    test1_init();
  }

  last_update = CLOCKCNT;
  refreshing = true;
  last_refresh = mscounter();
  test_begin_ms = mscounter();
}

void TMonoDisplayTest::Run()
{
  ++idlecnt;

  unsigned curms = mscounter();

  // By te monochrome displays the drawing happens in local buffer and then
  // the whole screen content transferred to the display, which may take longer time.
  // To avoid unnecessary transfers the refresh-es are limited to ~ 60 FPS

  if (curms - last_refresh >= REFRESH_TIME_MS)
  {
    last_refresh = curms;
    ++framecnt;

    textscreen.Run(); // draw the changed characters to the screen buffer

    refreshing = true;
  }

  if (refreshing)
  {
    if (!disp.UpdateFinished())  // disp.Run() called inside
    {
      return;
    }
    refreshing = false;
  }

#if !SINGLE_TEST
  if (curms - test_begin_ms > TEST_CHANGE_MS)
  {
    ++testid;
    if (testid >= TEST_COUNT)
    {
      testid = 0;
    }

    InitTest();
  }
#endif

  if (0 == testid)
  {
    test0_run();
  }
  else if (1 == testid)
  {
    test1_run();
  }
}

void TMonoDisplayTest::test0_init()
{
  disp.color = 1;
  disp.DrawRect(0, 0, disp.width, disp.height);

  disp.DrawLine(1, 1, disp.width-1, disp.height-1);

  disp.DrawLine(1, disp.height-1, disp.width-1, 1);

  disp.FillRect(disp.width-13, disp.height-13, 10, 10, 1);

  disp.SetCursor(3, 2);
  disp.printf("Hello from VIHAL!");
}

void TMonoDisplayTest::test0_run()
{
  disp.color = 1;
  disp.SetCursor(4, disp.height / 2);
  disp.printf("Idle cnt: %u ", idlecnt);
}

void TMonoDisplayTest::test1_init()
{
  disp.color = 1;
  //disp.DrawRect(0, 0, disp.width, disp.height);

  textscreen.InitCharBuf(TXTSCR_MAX_WIDTH, TXTSCR_MAX_HEIGHT, &txtscr_charbuf[0], &txtscr_changebuf[0]);  // init to the maximal buffer size
  textscreen.InitTextGfx(&disp, 0, 0, disp.width, disp.height, disp.font);

  textscreen.Clear();
  textscreen.Refresh();
}

void TMonoDisplayTest::test1_run()
{
  unsigned t;

  t = CLOCKCNT;

  textscreen.WriteChar(txtchar);
  ++txtchar;
  if (txtchar >= 120)  txtchar = 32;

#if 0
  textscreen.cursor_on = true;
  textscreen.cursor_x = textscreen.cposx;
  textscreen.cursor_y = textscreen.cposy;
#else
  textscreen.cursor_on = false;
#endif
}
