
#include "string.h"

#include "ff.h"

#include "board_pins.h"
#include "clockcnt.h"
#include "traces.h"

#include "mp_printf.h"

#include "uscounter.h"

#include "test_fatfs.h"

FATFS    g_fatfs;

uint8_t  databuf[DATABUF_SIZE]   __attribute__((aligned(4)));
uint8_t  databuf2[DATABUF_SIZE]  __attribute__((aligned(4)));


void display_bm_res(uint32_t ausecs, uint32_t abytesize)
{
  float ul_rw_speed = ((float)abytesize * 1000.0) / ausecs;
  TRACE("  Speed: %8.0f kB/s\n\r", ul_rw_speed);
}

void test_file_write()
{
  int i;
  FRESULT  fres;
  FIL   fil;
  UINT  rlen;

  //Try to open file
  fres = f_open(&fil, "1stfile.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if (fres != FR_OK)
  {
    TRACE("Error opening file to write!\r\n");
  }
  else
  {
    //If we put more than 0 characters (everything OK)

    for (i = 0; i < sizeof(databuf); ++i)
    {
      databuf[i] = 0xB0 + i;
    }

    fres = f_write(&fil, databuf, sizeof(databuf), &rlen);
    if (fres != FR_OK)
    {
      TRACE("File write ERROR: %i\r\n", fres);
    }
    else
    {
      TRACE("File write ok.\r\n");
    }

    f_close(&fil);
  }
}

bool create_required_dir(const char * adir)
{
  FRESULT  fres;
  FILINFO  finfo;
  fres = f_stat(adir, &finfo);
  if (FR_OK == fres)
  {
    return true;
  }

  if ((FR_NO_FILE == fres) || (FR_NO_PATH == fres))
  {
    TRACE("Creating \"%s\" subdirectory...\r\n", adir);
    fres = f_mkdir(adir);
    if (FR_OK == fres)
    {
      return true;
    }
    TRACE("Error creating \"%s\" subdirectory: %d\r\n", fres);
    return false;
  }
  TRACE("Error checking \"%s\" subdirectory: %d\r\n", fres);
  return false;
}

void cleanup_dir(const char * adir)
{
  FRESULT  fres;
  DIR      dinfo;
  FILINFO  finfo;
  char     fname[128];

  fres = f_findfirst(&dinfo, &finfo, adir, "*");
  while (fres == FR_OK && finfo.fname[0])
  {
    mp_snprintf(fname, sizeof(fname), "%s/%s", adir, finfo.fname);
    TRACE("  Deleting \"%s\"\r\n", fname);
    fres = f_unlink(fname);
    fres = f_findnext(&dinfo, &finfo);
  }

  f_closedir(&dinfo);
}

void test_bigfile_write()
{
  FRESULT  fres;
  UINT     rlen;
  FIL      fil;
  FILINFO  finfo;
  int      fi, chi, dwi;
  char     fname[128];

  uint32_t    data32;
  uint32_t *  pu32;
  uint32_t    dwcount;
  uint32_t    chunkcnt;

  uint32_t t0, t1;


  const char * testdir = "bigfiles";

  TRACE("Testing big file writes...\r\n");

  if (!create_required_dir(testdir))
  {
    return;
  }

  cleanup_dir(testdir);

  for (fi = 0; fi < BIGFILE_COUNT; ++fi)
  {
    mp_snprintf(fname, sizeof(fname), "%s/bigfile_longname_%i.bin", testdir, fi);

    TRACE("Writing \"%s\", bytes: %u ...\r\n", fname, BIGFILE_SIZE);

    //Try to open file
    fres = f_open(&fil, fname, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (fres != FR_OK)
    {
      TRACE("  File open error: %i\r\n", fres);
    }
    else
    {
      t0 = uscounter.Get32();

      data32 = (fi << 24);
      chunkcnt = BIGFILE_SIZE / sizeof(databuf);
      for (chi = 0; chi < chunkcnt; ++chi)
      {
        // prepare chunk data
        pu32 = (uint32_t *)&databuf[0];
        dwcount = (sizeof(databuf) >> 2);
        while (dwcount)
        {
          *pu32++ = data32++;
          --dwcount;
        }

        // write the chunk data

        fres = f_write(&fil, databuf, sizeof(databuf), &rlen);
        if (fres != FR_OK)
        {
          break;
        }
        else if (sizeof(databuf) != rlen)
        {
          fres = FRESULT(100);
          TRACE("  Incomplete write: %u != %u\r\n", rlen, sizeof(databuf));
          break;
        }
      } // chunk cycle

      if (fres)
      {
        TRACE("  Write error: %i\r\n", fres);
      }
      else
      {
        t1 = uscounter.Get32();
        //TRACE("  OK.\r\n");
        display_bm_res(t1-t0, BIGFILE_SIZE);
      }

      f_close(&fil);
    }
  }
}

void test_bigfile_read(bool averify)
{
  FRESULT  fres;
  UINT     rlen;
  FIL      fil;
  FILINFO  finfo;
  int      fi, chi, dwi;
  char     fname[128];

  // works like the bigfile write in order to check the file contents

  uint32_t    data32;
  uint32_t *  pu32;
  uint32_t    dwcount;
  uint32_t    chunkcnt;
  uint32_t    fcerrors;

  uint32_t t0, t1;

  const char * testdir = "bigfiles";

  if (averify)
  {
    TRACE("Testing big file reads with content check...\r\n");
  }
  else
  {
    TRACE("Testing big file read performance...\r\n");
  }

  for (fi = 0; fi < BIGFILE_COUNT; ++fi)
  {
    mp_snprintf(fname, sizeof(fname), "%s/bigfile_longname_%i.bin", testdir, fi);

    TRACE("Reading \"%s\", bytes: %u ...\r\n", fname, BIGFILE_SIZE);

    //Try to open file
    fres = f_open(&fil, fname, FA_OPEN_EXISTING | FA_READ);
    if (fres != FR_OK)
    {
      TRACE("  File open error: %i\r\n", fres);
    }
    else
    {
      t0 = uscounter.Get32();

      fcerrors = 0;
      data32 = (fi << 24);
      chunkcnt = BIGFILE_SIZE / sizeof(databuf);
      for (chi = 0; chi < chunkcnt; ++chi)
      {
        fres = f_read(&fil, databuf, sizeof(databuf), &rlen);
        if (fres != FR_OK)
        {
          break;
        }
        else if (sizeof(databuf) != rlen)
        {
          fres = FRESULT(100);
          TRACE("  Incomplete read: %u != %u\r\n", rlen, sizeof(databuf));
          break;
        }

        if (averify)
        {
          // verify the content
          pu32 = (uint32_t *)&databuf[0];
          dwcount = (sizeof(databuf) >> 2);
          while (dwcount)
          {
            if (*pu32++ != data32++)
            {
              ++fcerrors;
              break;
            }
            --dwcount;
          }
        }
      } // chunk cycle

      if (fres)
      {
        TRACE("  Read error: %i\r\n", fres);
      }
      else
      {
        t1 = uscounter.Get32();
        if (averify)
        {
          if (fcerrors)
          {
            TRACE("  Content errors: %u\r\n", fcerrors);
          }
          else
          {
            TRACE("  Content verify OK.\r\n");
          }
        }
        display_bm_res(t1-t0, BIGFILE_SIZE);
      }

      f_close(&fil);
    } // if opened
  } // for files
}


void test_fatfs()
{
  TRACE("FatFS test start\r\n");
  TRACE("Data buffer size: %u\r\n", sizeof(databuf));

  uscounter.Init();

  FRESULT  fres;
  //FIL fil;

  fres = f_mount(&g_fatfs, "", 1);  // last param: 1 = wait for mount
  if (fres != FR_OK)
  {
    TRACE("FatFS mount error: %i\r\n", fres);
    return;
  }

  TRACE("FatFS mount ok.\r\n");

  //test_file_write();

  //test_bigfile_write();
  //test_bigfile_read(true);
  test_bigfile_read(false);

  TRACE("Unmounting FatFs...\r\n");
  f_mount(0, "", 1);

  TRACE("FatFS test finished.\r\n");
}



