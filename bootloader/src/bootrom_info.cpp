// VRV100 bootrom info

#include "platform.h"

extern "C" void _reset_entry(void);

extern const bootrom_info_t bootrom_info;  // required so that the optimizer keeps this

__attribute__((section(".bootrom_info"),used))
const bootrom_info_t bootrom_info =
{
  .compid = 0x01530001,
  .bootblock_staddr = BOOTBLOCK_STADDR
};

