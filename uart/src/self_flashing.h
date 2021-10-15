/*
 * self_flashing.h
 *
 *  Created on: Oct 11, 2021
 *      Author: vitya
 */

#ifndef SRC_SELF_FLASHING_H_
#define SRC_SELF_FLASHING_H_

#include "platform.h"

#if defined(MCUF_VRV100)

#include "spiflash.h"

bool spi_self_flashing(TSpiFlash * spiflash);

#endif

#endif /* SRC_SELF_FLASHING_H_ */
