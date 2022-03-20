/*
 * netadapter.h
 *
 *  Created on: Mar 20, 2022
 *      Author: vitya
 */

#ifndef NETWORK_NETADAPTER_H_
#define NETWORK_NETADAPTER_H_

#include "stdint.h"
#include "network.h"
#include "hweth.h"

class TProtocolHandler
{
public:
  TProtocolHandler *  next = nullptr;

  virtual             ~TProtocolHandler() { } // never ment to be destructed, but the GCC requires this

public: // virtual functions
  virtual bool        HandleRxPacket(TPacketMem * apkt) { return false; }  // return true, if the packet is handled
  virtual void        Run() { }
};

// TNetAdapter could be the child of the THwEth too

class TNetAdapter
{
public: // settings

  uint8_t             max_rx_packets = 8;
  uint8_t             max_tx_packets = 8;

public:
  bool                initialized = false;
  THwEth *            peth = nullptr;

  TProtocolHandler *  firsthandler = nullptr;

  bool                Init(THwEth * aeth, void * anetmem, unsigned anetmemsize);

  void                Run(); // must be called regularly

  TPacketMem *        AllocateTxPacket();
  void                ReleaseTxPacket(TPacketMem * apmem);

  uint8_t *           AllocateNetMem(unsigned asize);
  int                 NetMemAvailable() { return netmem_size - netmem_allocated; }

  void                AddHandler(TProtocolHandler * ahandler);

protected: // internal memory management

  TPacketMem *        first_tx_pmem = nullptr;

  uint8_t *           rx_desc_mem = nullptr;
  uint8_t *           tx_desc_mem = nullptr;

  uint8_t *           rx_pmem = nullptr;
  uint8_t *           tx_pmem = nullptr;

  uint8_t *           netmem = nullptr;
  unsigned            netmem_size = 0;
  unsigned            netmem_allocated = 0;

};


#endif /* NETWORK_NETADAPTER_H_ */
