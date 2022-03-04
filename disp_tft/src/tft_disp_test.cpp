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
#include "tft_disp_test.h"

uint8_t txtscr_charbuf[128*64];
uint8_t txtscr_changebuf[sizeof(txtscr_charbuf) / 8];

TGfxTextScreen   textscreen;

TTftDisplayTest  displaytest;

#include "font_FreeSerifBold9pt7b.h"
TGfxFont font_serif(&FreeSerifBold9pt7b);

//#include "font_FreeMonoBold9pt7b.h"
//TGfxFont font_serif(&FreeMonoBold9pt7b);

void tft_disp_test_run()
{
  displaytest.Run();
}

void tft_disp_test_init()
{
	TRACE("--- TFT Display Test ---\r\n");

	displaytest.Init();
}

void TTftDisplayTest::Init()
{
  testid = START_TEST;
  InitTest();

  test_begin_ms = mscounter();
}

void TTftDisplayTest::InitTest()
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
  else if (2 == testid)
  {
    test2_init();
  }

  last_update = CLOCKCNT;
  test_begin_ms = mscounter();
}

void TTftDisplayTest::Run()
{
  unsigned curms = mscounter();

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
  else if (2 == testid)
  {
    test2_run();
  }
}

void TTftDisplayTest::test0_init()
{
  pos_y = 10;
  pos_x = 10;
  dwidth = disp.width / 3;
  dheight = disp.height - 20;

  disp.color = RGB16(0, 255, 0);
  disp.FillRect(10, 10, 100, 100, disp.color);

  disp.color = RGB16(255, 0, 0);
  disp.DrawRect(0, 0, disp.width, disp.height);
}

void TTftDisplayTest::test0_run()
{
  unsigned t;

  t = CLOCKCNT;
  if (t - last_update > SystemCoreClock / 60)
  {
    int16_t oldpos = pos_x;

    ++pos_x;
    if (pos_x > disp.width - dwidth - 10)
    {
      pos_x = 10;
    }
    disp.color = RGB16(0, 255, 0);
    disp.FillRect(pos_x, 10, dwidth, dheight, disp.color);

    // delete the old part
    disp.color = RGB16(0, 0, 0);
    if (oldpos < pos_x)
    {
      disp.FillRect(oldpos, 10, 1, dheight, disp.color);
    }
    else
    {
      disp.FillRect(oldpos, 10, dwidth, dheight, disp.color);
    }

    disp.color = RGB16(255, 255, 255);
    disp.bgcolor = 0;
    disp.SetCursor(10, 10);
    disp.printf("Frame: %09u", framecnt);

    disp.color = 0xffff;
    disp.SetFont(&font_serif);
    disp.SetCursor(20, disp.height / 2);
    disp.DrawString("Hello World!");

    disp.SetFont(&font_gfx_standard);

    ++framecnt;

    last_update = t;
  }
}

void TTftDisplayTest::test1_init()
{
  disp.color = RGB16(255, 255, 0);
  disp.DrawRect(0, 0, disp.width, disp.height);

  disp.color = RGB16(0, 0, 255);
  disp.DrawLine(1, 1, disp.width-1, disp.height-1);

  disp.color = RGB16(0, 255, 0);
  disp.DrawLine(1, disp.height-1, disp.width-1, 1);

  disp.color = 0xffff;
  disp.SetCursor(10, 10);
  disp.DrawString("This is the test 2.");
}

void TTftDisplayTest::test1_run()
{
  // no movement here
}

void TTftDisplayTest::test2_init()
{
  disp.color = RGB16(255, 255, 0);
  disp.DrawRect(0, 0, disp.width, disp.height);

  textscreen.InitCharBuf(128, 64, &txtscr_charbuf[0], &txtscr_changebuf[0]);  // init to the maximal buffer size
  textscreen.InitTextGfx(&disp, 3, 3, disp.width-7, disp.height-7, disp.font);

  disp.color = RGB16(255, 255, 255);

  textscreen.Clear();
  textscreen.Refresh();
}

void TTftDisplayTest::test2_run()
{
  unsigned t;

  t = CLOCKCNT;
  if (t - last_update > SystemCoreClock / 1000)
  {
    textscreen.WriteChar(txtchar);
    ++txtchar;
    if (txtchar >= 120)  txtchar = 32;

    ++framecnt;

    last_update = t;
  }

  textscreen.Run();
}
