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

//--------------------------------------------------------------

void TIp4Handler::Init(TNetAdapter *aadapter)
{
  adapter = aadapter;

  syspkt = adapter->AllocateTxPacket();  // reserve one TX packet for system purposes

  adapter->AddHandler(this);
}

void TIp4Handler::Run()
{
}

bool TIp4Handler::HandleRxPacket(TPacketMem * apkt)  // return true, if the packet is handled
{
  return false;
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
