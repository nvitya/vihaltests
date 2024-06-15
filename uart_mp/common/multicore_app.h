/*
 * multicore_app.h
 *
 *  Created on: Jun 15, 2024
 *      Author: vitya
 */

#ifndef COMMON_MULTICORE_APP_H_
#define COMMON_MULTICORE_APP_H_

#define PRIMARY_CODE_ADDR          0x21000000
#define PRIMARY_CODE_SIZE_MAX         0x10000

#define SECONDARY_SELF_FLASH_FLAG           1

#define SECONDARY_CODE_ADDR        0x21020000
#define SECONDARY_CODE_SIZE_MAX       0x10000
#define SECONDARY_CODE_SPI_ADDR      0x030000  // start at 192k
#define SECONDARY_STACK_ADDR       (0x21040000 - 4)

#endif /* COMMON_MULTICORE_APP_H_ */
