

MEMORY
{
  FLASH (rx) :    ORIGIN = 0x08000000, LENGTH = 64K

  ITCRAM (rwx) :  ORIGIN = 0x00000000, LENGTH = 16K
  DTCRAM (xrw) :  ORIGIN = 0x20000000, LENGTH = 64K
}

REGION_ALIAS("TEXT", FLASH);
REGION_ALIAS("RAM",  DTCRAM);
REGION_ALIAS("RAM2", DTCRAM);

INCLUDE armm_default_flash.ld

