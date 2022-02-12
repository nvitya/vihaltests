/*
 * i2cmanager.cpp
 *
 *  Created on: Feb 12, 2022
 *      Author: vitya
 */

#include <i2cmanager.h>

bool TI2cManager::Init(THwI2c * ai2c)
{
  pi2c = ai2c;
  curtra = nullptr;
  state = 0;

  return true;
}

void TI2cManager::Run()
{
  if (!pi2c)
  {
    return;
  }

  pi2c->Run();

  if (!curtra)
  {
    return;
  }

  if (0 == state)
  {
    if (curtra->iswrite)
    {
      if (!pi2c->StartWriteData(curtra->address, curtra->extra, curtra->dataptr, curtra->datalen))
      {
        curtra->errorcode = pi2c->error;
        curtra->completed = true;
        state = 10;
      }
    }
    else
    {
      if (!pi2c->StartReadData(curtra->address, curtra->extra, curtra->dataptr, curtra->datalen))
      {
        curtra->errorcode = pi2c->error;
        curtra->completed = true;
        state = 10;
      }
    }

    if (state != 10) // go on closing the transaction
    {
      state = 1;  // wait for complete
      return;
    }
  }
  else if (1 == state) // wait for finish
  {
    if (pi2c->busy)
    {
      return;
    }

    curtra->errorcode = pi2c->error;
    state = 10;
  }

  if (10 == state)  // request completed, remove from the queue
  {
    // the callback function might add the same transaction object as new
    // therefore we have to remove the transaction from the chain before we call the callback
    TI2cTransaction * ptra = curtra; // save the transaction pointer for the callback

    curtra->completed = true;
    curtra = curtra->next; // advance to the next transaction
    state = 0;

    // call the callback
    PCbClassCallback pcallback = PCbClassCallback(ptra->callback);
    if (pcallback)
    {
      TCbClass * obj = (TCbClass *)(ptra->callbackobj);
      (obj->*pcallback)(ptra->callbackarg);
    }
  }
}

void TI2cManager::AddWrite(TI2cTransaction * atra, uint8_t aaddr, unsigned aextra, void * dataptr, unsigned len)
{
  atra->iswrite = 1;
  atra->address = aaddr;
  atra->extra = aextra;
  atra->dataptr = (uint8_t *)dataptr;
  atra->datalen = len;

  AddTransaction(atra);
}

void TI2cManager::AddRead(TI2cTransaction * atra, uint8_t aaddr, unsigned aextra, void * dataptr, unsigned len)
{
  atra->iswrite = 0;
  atra->address = aaddr;
  atra->extra = aextra;
  atra->dataptr = (uint8_t *)dataptr;
  atra->datalen = len;

  AddTransaction(atra);
}

void TI2cManager::AddTransaction(TI2cTransaction * atra)
{
  atra->next = nullptr;
  atra->completed = false;
  atra->errorcode = 0;

  if (curtra)
  {
    // search the last tra
    TI2cTransaction * tra = curtra;
    while (tra->next)
    {
      tra = tra->next;
    }
    tra->next = atra;
  }
  else
  {
    // set as first
    curtra = atra;
  }
}

void TI2cManager::WaitFinish(TI2cTransaction * atra)
{
  while (!atra->completed)
  {
    Run();
  }
}
