/*
 * test_common.h
 *
 *  Created on: Apr 17, 2024
 *      Author: vitya
 */

#ifndef SRC_TEST_COMMON_H_
#define SRC_TEST_COMMON_H_

#include "platform.h"

uint8_t * sdram_alloc(unsigned asize);
uint32_t sdram_free_bytes();

bool sdcard_init_wait(unsigned ausecs);

void display_bm_res(uint32_t ausecs, uint32_t abytesize);

void show_mem(void * addr, unsigned len);


#endif /* SRC_TEST_COMMON_H_ */
