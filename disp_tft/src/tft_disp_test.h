// file:     tft_disp_test.cpp
// brief:    TFT LCD Display Test
// created:  2022-03-04
// authors:  nvitya

#ifndef SRC_TFT_DISP_TEST_H_
#define SRC_TFT_DISP_TEST_H_

#include "gfxbase.h"
#include "gfxtextscreen.h"

#define TEST_CHANGE_MS  8000

#define TEST_COUNT   3
#define START_TEST   0
#define SINGLE_TEST  0

class TTftDisplayTest
{
public:
  unsigned    framecnt = 0;
  unsigned    idlecnt = 0;

  unsigned    test_begin_ms = 0;

  int         testid = -1;
  int16_t     pos_x = 0;
  int16_t     pos_y = 0;

  unsigned    last_update = 0;

  int16_t     dwidth = 200;
  int16_t     dheight = 240;

  char        txtchar = 33;

  TGfxBase *  pdisp = nullptr;

  void Init();
  void InitTest();
  void Run();

  void test0_init();
  void test0_run();

  void test1_init();
  void test1_run();

  void test2_init();
  void test2_run();
};

void tft_disp_test_init();
void tft_disp_test_run();

extern TTftDisplayTest  displaytest;

#endif /* SRC_TFT_DISP_TEST_H_ */
