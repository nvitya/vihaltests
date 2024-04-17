/*
 * test_sdcard_fs.cpp
 */

#include "string.h"

#include "stdlib.h"
#include "board_pins.h"
#include "test_sdcard_fs.h"
#include "clockcnt.h"
#include "hwuscounter.h"
#include "traces.h"
#include "test_common.h"
#include "stormanager.h"
#include "storman_sdcard.h"

#include "filesystem_types.h"
#include "filesys_fat.h"
#include "fileman.h"

static uint32_t databuf_size = 0;

static uint8_t * databuf;
static uint8_t * databuf2;

TStorManSdcard  sm_sdcard;
int dev_sdcard = -1;
TFileSysFat     fatfs;
TFile *         pfile;


TStorTrans  stra;

TMbrPtEntry     ptable[4];
uint32_t        fs_firstsector;
uint32_t        fs_maxsectors;

void init_filesystems()
{
  unsigned n;

  TRACE("Initializing File Systems...\r\n");

  fs_firstsector = 0;
  fs_maxsectors = 0;

  TRACE("Reading SDCARD partition table...\r\n");

  sm_sdcard.AddTransaction(&stra, STRA_READ, 446, &ptable[0], 64);
  sm_sdcard.WaitTransaction(&stra);
  if (stra.errorcode)
  {
    TRACE("Error reading partition table!\r\n");
  }
  else
  {
    TRACE("SDCARD Partition table:\r\n");
    for (n = 0; n < 4; ++n)
    {
      if ((fs_firstsector == 0) && (ptable[n].ptype != 0) && ptable[n].first_lba)
      {
        fs_firstsector = ptable[n].first_lba;
        fs_maxsectors = ptable[n].sector_count;
      }

      TRACE(" %u.: status=%02X, type=%02X, start=%08X, blocks=%i\r\n",
          n, ptable[n].status, ptable[n].ptype, ptable[n].first_lba, ptable[n].sector_count
      );
    }
  }

  if (fs_maxsectors)
  {
    TRACE("Initializing FAT FS at sector %i...\r\n", fs_firstsector);

    fatfs.Init(&sm_sdcard, (fs_firstsector << 9), (fs_maxsectors << 9));
    while (!fatfs.initialized)
    {
      fatfs.Run();
    }

    if (fatfs.fsok)
    {
      TRACE("FAT file system initialized:\r\n");
      if (fatfs.fat32)  TRACE(" FAT32\r\n");
      TRACE(" cluster size: %u\r\n", fatfs.clusterbytes);
      TRACE(" total size: %u MByte\r\n", fatfs.databytes >> 20);

      fileman.AddFileSystem(&fatfs);
    }
  }
}

int      g_dir_maxdepth = 4;
int      g_dir_depth = 0;
char     g_ident[128];

void set_dir_depth(int adepth)
{
  g_dir_depth = adepth;
  int i = 0;
  while (i < g_dir_depth)
  {
    g_ident[i] = ' ';
    ++i;
  }
  g_ident[i] = 0;
}

void list_directory(const char * adirpath) //uint64_t adirstart)
{
  int            i;
  TFile *        dfile;  // allocated on the heap !
  TFileDirData   fdata;

  dfile = fileman.NewFileObj("", nullptr, 0);  // get a file object for the first filesystem
  if (!dfile)
  {
    return;
  }

  set_dir_depth(g_dir_depth + 1);

  dfile->Open(adirpath, FOPEN_DIRECTORY);
  if (0 != dfile->WaitComplete())
  {
    TRACE("Error(%i) opening directory at \"%s\".\r\n", dfile->result, adirpath);
  }
  else
  {
    TRACE("Directory opened successfully.\r\n");
    while (1)
    {
      dfile->Read(&fdata, sizeof(fdata));
      if (0 != dfile->WaitComplete())
      {
        break;
      }

      if (fdata.attributes & FSATTR_DIR)
      {
        if ((strcmp(".", fdata.name) != 0) && (strcmp("..", fdata.name) != 0))
        {
          TRACE("%s [%s]\r\n", g_ident, fdata.name);
          if (g_dir_depth < g_dir_maxdepth)
          {
            char newpath[FS_PATH_MAX_LEN];
            strncpy(newpath, adirpath, sizeof(newpath));
            unsigned len = strlen(newpath);
            if (('/' != newpath[len-1]) && ('\\' != newpath[len-1]))
            {
              strcat(newpath, "/");
            }
            strcat(newpath, fdata.name);

            list_directory(newpath);
          }
        }
      }
      else if (0 == (fdata.attributes & FSATTR_NONFILE))
      {
        TRACE("%s %s: %u bytes\r\n", g_ident, fdata.name, uint32_t(fdata.size));
      }
    }
  }

  set_dir_depth(g_dir_depth - 1);
  fileman.ReleaseFileObj(dfile);
}

void test_dir_read(const char * adirstart, unsigned amaxdepth)
{
  TRACE("Testing directory read at \"%s\", depth=%u\r\n", adirstart, amaxdepth);

  g_dir_maxdepth = amaxdepth;
  g_dir_depth = 0;

  list_directory(adirstart);
}

void test_file_read(const char * aname)
{
  TRACE("Testing file open \"%s\"...\r\n", aname);

  pfile->Open(aname, 0);
  pfile->WaitComplete();

  TRACE("File Open Result: %i\r\n", pfile->result);
  if (0 == pfile->result)
  {
    TRACE("  file size: %llu\r\n", pfile->fdata.size);
    TRACE("  location:  %llu\r\n", pfile->fdata.location);

    TRACE("Reading file...\r\n");
    pfile->Read(&databuf[0], databuf_size);
    pfile->WaitComplete();

    TRACE("File Read Result: %i\r\n", pfile->result);
    if (0 == pfile->result)
    {
      TRACE(" transferred length: %u\r\n", pfile->transferlen);

#if 0
      int i = 0;
      char * cp = (char *)&databuf[0];
      while (i < 4000)
      {
        if (*cp == 10)  TRACE("\r");
        TRACE("%c", *cp);
        ++cp;
        ++i;
      }
#endif
    }
  }
}

#if 0
void test_file_seek(const char * aname, uint64_t astart, uint32_t alen)
{
  TRACE("Testing file Seek(%llu, %u) \"%s\"...\r\n", astart, alen, aname);

  pfile->Open(aname, 0);
  if (0 != pfile->WaitComplete())
  {
    TRACE("File open error: %i\r\n", pfile->result);
    return;
  }

  pfile->Seek(astart);
  if (0 != pfile->WaitComplete())
  {
    TRACE("File seek error: %i\r\n", pfile->result);
    return;
  }

  pfile->Read(&filebuf[0], alen);
  if (0 != pfile->WaitComplete())
  {
    TRACE("File read error: %i\r\n", pfile->result);
    return;
  }

  TRACE("Part 1 ok, Repeating seek+read...\r\n");

  pfile->Seek(astart);
  if (0 != pfile->WaitComplete())
  {
    TRACE("File seek2 error: %i\r\n", pfile->result);
    return;
  }

  pfile->Read(&filebuf2[0], alen);
  if (0 != pfile->WaitComplete())
  {
    TRACE("File read2 error: %i\r\n", pfile->result);
    return;
  }

  TRACE("Part2 finished, comparing results...\r\n");

  if (0 != memcmp(filebuf, filebuf2, alen))
  {
    TRACE("Error: result data difference!\r\n");
  }
  else
  {
    TRACE("Result data ok.\r\n");
  }
}
#endif

void test_sdcard_fs()
{
  int i;

  TRACE("SD-Card file system test\r\n");

  if (hwsdram.initialized)
  {
    databuf_size = hwsdram.byte_size / 2;
    databuf  = (uint8_t *)sdram_alloc(databuf_size);
    databuf2 = (uint8_t *)sdram_alloc(databuf_size);
  }
  else
  {
    databuf_size = 16 * 1024;
    databuf  = (uint8_t *)malloc(databuf_size);
    databuf2 = (uint8_t *)malloc(databuf_size);
  }

  TRACE("Test buffer size: %u\r\n", databuf_size);

  if (!databuf or !databuf2)
  {
    TRACE("Test buffer allocation failed!\r\n");
    return;
  }

  if (!sdcard_init_wait(3000000))
  {
    return;  // error message is already printed.
  }

  sm_sdcard.Init(&sdcard);
  dev_sdcard = g_storman.AddManager(&sm_sdcard);
  if (dev_sdcard < 0)
  {
    TRACE("Error Adding SD-Card storage manager!\r\n");
    return;
  }

  TRACE("SD-Card storage manager initialized.\r\n");

  init_filesystems();

  pfile = fileman.NewFileObj("", nullptr, 0);  // get a file object for the first filesystem
  if (!pfile)
  {
    TRACE("Error allocating a file object, no active filesystems.\r\n");
    return;
  }

  TRACE("File object allocated successfully.\r\n");

  test_dir_read("/", 2);

  test_file_read("/BIGFILES/BIGFIL~1.BIN");

  TRACE("SDCard file system test finished.\r\n");
}
