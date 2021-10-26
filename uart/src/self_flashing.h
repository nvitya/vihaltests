/*
 * self_flashing.h
 *
 *  Created on: Oct 11, 2021
 *      Author: vitya
 */

#ifndef SRC_SELF_FLASHING_H_
#define SRC_SELF_FLASHING_H_

#include "platform.h"

#if SPI_SELF_FLASHING

#include "spiflash.h"

bool spi_self_flashing(TSpiFlash * spiflash);

#endif

#endif /* SRC_SELF_FLASHING_H_ */
