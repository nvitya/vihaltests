/*
 * i2cmanager.h
 *
 *  Created on: Feb 12, 2022
 *      Author: vitya
 */

#ifndef SRC_I2CMANAGER_H_
#define SRC_I2CMANAGER_H_

#include "stdint.h"
#include "hwi2c.h"

// class for scheduleing I2C transactions for the same I2C port

class TCbClass { };
typedef void (TCbClass::*PCbClassCallback)(void * arg);

struct TI2cTransaction
{
  bool               completed;
  uint8_t            iswrite;  // 0 = read, 1 = write
  uint8_t            address;
  int                errorcode;

  uint32_t           extra;
  uint8_t *          dataptr;
  unsigned           datalen;

  PCbClassCallback   callback = nullptr;
  void *             callbackobj = nullptr;
  void *             callbackarg = nullptr;

  TI2cTransaction *  next;
};


class TI2cManager
{
public:
  THwI2c *           pi2c = nullptr;
  int                state = 0;

  TI2cTransaction *  curtra = nullptr;

  bool               Init(THwI2c * ai2c);
  void               Run();

  void AddWrite(TI2cTransaction * atra, uint8_t aaddr, uint32_t aextra, void * dataptr, unsigned len);
  void AddRead(TI2cTransaction * atra, uint8_t aaddr, uint32_t aextra, void * dataptr, unsigned len);

  void WaitFinish(TI2cTransaction * atra);

protected:

  void AddTransaction(TI2cTransaction * atra);

};

#endif /* SRC_I2CMANAGER_H_ */
