/*
 * net_ip4.cpp
 *
 *  Created on: Mar 19, 2022
 *      Author: vitya
 */

#include "platform.h"
#include "net_ip4.h"
#include "traces.h"

uint16_t calc_udp4_checksum(TIp4Header * piph, uint16_t datalen)
{
  uint32_t n;
  uint32_t sum = 0;
  TUdp4Header * pudp = PUdp4Header(piph + 1); // the UDP header comes after the IP header

  // add the two IP addresses first
  uint16_t * pd16 = (uint16_t *)&piph->srcaddr;
  for (n = 0; n < 4; ++n)
  {
    sum += __REV16(*pd16++);
  }

  sum += piph->protocol; // add the protocol as well
  sum += __REV16(pudp->len); // add the UDP length

  // add the UDP header parts exlusive the checksum
  pd16 = (uint16_t *)pudp;
  for (n = 0; n < 3; ++n)
  {
    sum += __REV16(*pd16++);
  }

  // and then the data
  pd16 = (uint16_t *)(pudp + 1);
  for (n = 0; n < (datalen >> 1); ++n)
  {
    sum += __REV16(*pd16++);
  }

  if (datalen & 1)
  {
    sum += (*pd16 & 0xFF);  // mo byte swapping here, we need just the low byte
  }

  while (sum >> 16)
  {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  return (uint16_t) (~sum);
}

void TNetAdapter::Init(THwEth * aeth, void * anetmem, unsigned anetmemsize)
{
  peth = aeth;
  netmem = (uint8_t *)anetmem;
  netmem_size = anetmemsize;

  initialized = false;

  // memory allocation
  netmem_allocated = 0;
  // RX and TX descriptors
  rx_desc_mem = &netmem[netmem_allocated];
  netmem_allocated += sizeof(HW_ETH_DMA_DESC) * max_rx_packets;
  tx_desc_mem = &netmem[netmem_allocated];
  netmem_allocated += sizeof(HW_ETH_DMA_DESC) * max_tx_packets;

  rx_pmem = &netmem[netmem_allocated];
  netmem_allocated += sizeof(TPacketMem) * max_rx_packets;
  tx_pmem = &netmem[netmem_allocated];
  netmem_allocated += sizeof(TPacketMem) * max_tx_packets;

  if (netmem_allocated > netmem_size)
  {
    TRACE("Network memory is too small: %u, required: %u\r\n", netmem_size, netmem_allocated);
    return;
  }

  peth->promiscuous_mode = false;
  peth->hw_ip_checksum = false; //true;

  if (!peth->Init(rx_desc_mem, max_rx_packets, tx_desc_mem, max_tx_packets))
  {
    TRACE("ETH INIT FAILED!\r\n");
    return;
  }

  // Assign RX packet buffers, required for the receive

  for (unsigned n = 0; n < max_rx_packets; ++n)
  {
    peth->AssignRxBuf(n, &rx_pmem[sizeof(TPacketMem) * n + NETIF_PMEM_HEAD_SIZE], HWETH_MAX_PACKET_SIZE);
  }

  // Initialize TX packet allocation
  TPacketMem *  pmem = (TPacketMem *)tx_pmem;
  TPacketMem *  prevpmem = pmem;
  first_tx_pmem = pmem;
  for (unsigned n = 1; n < max_tx_packets; ++n)
  {
    ++pmem;
    prevpmem->next = pmem;
    pmem->next = nullptr;
    prevpmem = pmem;
  }

  syspkt = AllocateTxPacket();  // reserve one TX packet for system purposes

  // start the network interface

  peth->Start();

  initialized = true;
}

void TNetAdapter::Run()
{
  if (peth)
  {
    peth->PhyStatusPoll(); // must be called regularly
  }
}

TPacketMem * TNetAdapter::AllocateTxPacket()
{
  if (first_tx_pmem)
  {
    TPacketMem * result = first_tx_pmem;
    first_tx_pmem = first_tx_pmem->next;
    return result;
  }
  else
  {
    return nullptr;
  }
}

void TNetAdapter::ReleaseTxPacket(TPacketMem * apmem)
{
  apmem->next = first_tx_pmem;
  first_tx_pmem = apmem;
}

//--------------------------------------------------------------

void TUdp4Socket::Init(void * anif, uint16_t aport)
{
}

int TUdp4Socket::Send(void * adataptr, unsigned adatalen)
{
  return -1;
}

int TUdp4Socket::Receive(void * adataptr, unsigned adatalen)
{
  return -1;
}

void TIp4Handler::Init(TNetAdapter *aadapter)
{
}
