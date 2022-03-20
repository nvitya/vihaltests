/*
 * netadapter.cpp
 *
 *  Created on: Mar 20, 2022
 *      Author: vitya
 */


#include "netadapter.h"
#include "traces.h"

bool TNetAdapter::Init(THwEth * aeth, void * anetmem, unsigned anetmemsize)
{
  peth = aeth;
  netmem = (uint8_t *)anetmem;
  netmem_size = anetmemsize;

  initialized = false;
  firsthandler = nullptr;
  first_sending_pkt = nullptr;
  last_sending_pkt = nullptr;

  // memory allocation
  netmem_allocated = 0;
  // RX and TX descriptors
  rx_desc_mem = AllocateNetMem(sizeof(HW_ETH_DMA_DESC) * max_rx_packets);
  tx_desc_mem = AllocateNetMem(sizeof(HW_ETH_DMA_DESC) * max_tx_packets);
  if (!rx_desc_mem || !tx_desc_mem)
  {
    TRACE("NetAdapter: Error allocating RX/TX descriptors!\r\n");
    return false;
  }

  rx_pmem = AllocateNetMem(sizeof(TPacketMem) * max_rx_packets);
  if (!rx_pmem)
  {
    TRACE("NetAdapter: Error allocating RX packet buffers!\r\n");
    return false;
  }

  tx_pmem = AllocateNetMem(sizeof(TPacketMem) * max_tx_packets);
  if (!tx_pmem)
  {
    TRACE("NetAdapter: Error allocating TX packet buffers!\r\n");
    return false;
  }

  peth->promiscuous_mode = false;
  peth->hw_ip_checksum = false; //true;

  if (!peth->Init(rx_desc_mem, max_rx_packets, tx_desc_mem, max_tx_packets))
  {
    TRACE("NetAdapter: ETH INIT FAILED!\r\n");
    return false;
  }

  // Assign RX packet buffers, required for the receive

  for (unsigned n = 0; n < max_rx_packets; ++n)
  {
    peth->AssignRxBuf(n, (TPacketMem *)&rx_pmem[sizeof(TPacketMem) * n], HWETH_MAX_PACKET_SIZE);
  }

  // Initialize TX packet allocation
  TPacketMem *  pmem = (TPacketMem *)tx_pmem;
  TPacketMem *  prevpmem = pmem;
  first_free_tx_pmem = pmem;
  for (unsigned n = 1; n < max_tx_packets; ++n)
  {
    ++pmem;
    prevpmem->next = pmem;
    pmem->next = nullptr;
    prevpmem = pmem;
  }

  // start the network interface

  peth->Start();

  initialized = true;
  return true;
}

void TNetAdapter::AddHandler(TProtocolHandler * ahandler)
{
  ahandler->next = nullptr;
  if (!firsthandler)
  {
    firsthandler = ahandler;
  }
  else
  {
    TProtocolHandler * ph = firsthandler;
    while (ph->next)
    {
      ph = ph->next;
    }
    ph->next = ahandler;
  }
}

uint8_t * TNetAdapter::AllocateNetMem(unsigned asize)
{
  if (asize > NetMemAvailable())
  {
    return nullptr;
  }

  uint8_t * result = &netmem[netmem_allocated];
  netmem_allocated += asize;

  return result;
}

void TNetAdapter::Run()
{
  TPacketMem *        pmem;
  TProtocolHandler *  ph;

  if (peth)
  {
    peth->PhyStatusPoll(); // must be called regularly
  }

  // free Sended Tx Packets
  // TODO: check the whole chain ?
  while (first_sending_pkt && peth->SendFinished(first_sending_pkt->idx))
  {
    pmem = first_sending_pkt;
    first_sending_pkt = first_sending_pkt->next; // unchain first before free !

    ReleaseTxPacket(pmem);
  }
  if (!first_sending_pkt)
  {
    last_sending_pkt = nullptr;
  }

  // check for Rx Packets

  if (peth->TryRecv(&pmem))
  {
    pmem->flags = 0;

    ph = firsthandler;
    while (ph)
    {
      if (ph->HandleRxPacket(pmem))  // the protocol handler must release the TPacketMem !
      {
        break;
      }
      ph = ph->next;
    }

    if (!ph)
    {
      // the packet was not handled
    }

    if (0 == (pmem->flags & PMEMFLAG_KEEP))
    {
      // the packet was not handled, just ignore and release
      peth->ReleaseRxBuf(pmem);
    }
  }

  // Run Idle parts

  ph = firsthandler;
  while (ph)
  {
    ph->Run();
    ph = ph->next;
  }
}

TPacketMem * TNetAdapter::AllocateTxPacket()
{
  if (first_free_tx_pmem)
  {
    TPacketMem * result = first_free_tx_pmem;
    first_free_tx_pmem = first_free_tx_pmem->next;
    return result;
  }
  else
  {
    return nullptr;
  }
}

void TNetAdapter::ReleaseTxPacket(TPacketMem * apmem)
{
  apmem->next = first_free_tx_pmem;
  first_free_tx_pmem = apmem;
}

bool TNetAdapter::SendTxPacket(TPacketMem * apmem)  // the packet will be automatically released
{
  // send the packet on the Ethernet

  uint32_t idx;
  if (!peth->TrySend(&idx, &apmem->data[0], apmem->datalen))
  {
    ReleaseTxPacket(apmem);
    return false;
  }

  // add to the sending chain
  apmem->idx = idx;
  apmem->next = nullptr;

  TPacketMem * pmem;
  if (last_sending_pkt)
  {
    last_sending_pkt->next = apmem;
    last_sending_pkt = apmem;
  }
  else
  {
    first_sending_pkt = apmem;
    last_sending_pkt = apmem;
  }

  return true;
}
