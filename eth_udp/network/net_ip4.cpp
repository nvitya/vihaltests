/*
 * net_ip4.cpp
 *
 *  Created on: Mar 19, 2022
 *      Author: vitya
 */

#include "string.h"
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

void TArp4Table::Init(TProtocolHandler * ahandler)
{
  phandler = ahandler;
  firstitem = nullptr;
  lastitem = nullptr;
  freelist = nullptr;

  // allocate the arp table
  TArp4TableItem *  items =
      (TArp4TableItem *) phandler->adapter->AllocateNetMem(sizeof(TArp4TableItem) * max_items);

  // fill the free list
  for (int i = max_items - 1; i >= 0; --i)  // make it reverse to be linear in the memory (cache optimization)
  {
    TArp4TableItem *  item = &items[i];
    item->next = freelist;
    item->prev = nullptr; // not used here
    freelist = item;
  }
}

TArp4TableItem * TArp4Table::CreateNewItem()
{
  TArp4TableItem *  item;

  if (freelist)
  {
    item = freelist;
    freelist = freelist->next;
  }
  else
  {
    // take the last (oldest) item
    item = lastitem;

    lastitem = lastitem->prev;
    if (lastitem)
    {
      lastitem->next = nullptr;
    }
    else
    {
      firstitem = nullptr; // (impossible case)
    }
  }

  item->prev = nullptr;
  item->next = nullptr;
  return item;
}

void TArp4Table::Update(TIp4Addr aipaddr, uint8_t * amacaddr)
{
  TArp4TableItem *  item = FindByMac(amacaddr);

  if (item)
  {
    // just move to forward
    item->ipaddr = aipaddr;

    // unchain
    if (item->prev)   item->prev->next = item->next;
    else              firstitem = item->next;

    if (item->next)   item->next->prev = item->prev;
    else              lastitem = item->prev;
  }
  else
  {
    item = CreateNewItem();
  }

  // add as first

  item->next = firstitem;
  item->prev = nullptr;
  if (firstitem)
  {
    firstitem->prev = item;
  }
  firstitem = item;
  if (!lastitem)
  {
    lastitem = item;
  }
}

TArp4TableItem * TArp4Table::FindByIp(TIp4Addr aipaddr)
{
  TArp4TableItem *  item = firstitem;
  while (item)
  {
    if (item->ipaddr.u32 == aipaddr.u32)
    {
      return item;
    }
    item = item->next;
  }
  return nullptr;
}

TArp4TableItem * TArp4Table::FindByMac(uint8_t * amacaddr)
{
  TArp4TableItem *  item = firstitem;
  while (item)
  {
    if (    (*(uint32_t *)&item->macaddr[0] == *(uint32_t *)&amacaddr[0])
         && (*(uint16_t *)&item->macaddr[4] == *(uint16_t *)&amacaddr[4]) )
    {
      return item;
    }
    item = item->next;
  }
  return nullptr;
}

//--------------------------------------------------------------

void TUdp4Socket::Init(TIp4Handler * ahandler, uint16_t alistenport)
{
  phandler = ahandler;
  listenport = alistenport;

  phandler->AddUdpSocket(this);
}

int TUdp4Socket::Send(void * adataptr, unsigned adatalen)
{
  TPacketMem * pmem;

  pmem = phandler->adapter->AllocateTxPacket();
  if (!pmem)
  {
    return 0;  // no free packet !
  }

  // assemble the UDP packet

  // TODO: assemble the UDP package!

  // does it require ARP ?
  TArp4TableItem * arpitem = phandler->arptable.FindByIp(destaddr);
  if (arpitem)
  {
    // update the destination MAC (this is the first field
    *(uint32_t *)&pmem->data[0] = *(uint32_t *)&arpitem->macaddr[0];
    *(uint16_t *)&pmem->data[4] = *(uint16_t *)&arpitem->macaddr[4];

    if (phandler->adapter->SendTxPacket(pmem))
    {
      return adatalen;
    }
    else
    {
      return 0;  // something was not succesful at the ETH HW level
    }
  }
  else // ARP required!
  {
    //TODO: request ARP

    return -1;
  }
}

int TUdp4Socket::Receive(void * adataptr, unsigned adatalen)
{
  TPacketMem * pmem = rxpkt_first;

  if (!pmem)
  {
    return 0;
  }

  // TODO: copy the UDP data from the packet to the user

  unsigned len = 1;

  rxpkt_first = rxpkt_first->next;
  if (!rxpkt_first)  rxpkt_last = nullptr;

  // release the packet
  phandler->adapter->ReleaseRxPacket(pmem);

  return len;
}

//--------------------------------------------------------------

void TIp4Handler::Init(TNetAdapter * aadapter)
{
  adapter = aadapter;

  udp_first = nullptr;
  udp_last  = nullptr;

  arptable.Init(this);

  syspkt = adapter->AllocateTxPacket();  // reserve one TX packet for system purposes

  adapter->AddHandler(this);
}

void TIp4Handler::AddUdpSocket(TUdp4Socket * audp)
{
  if (udp_last)
  {
    udp_last->nextsocket = audp;
  }
  else
  {
    udp_last = audp;
    udp_first = audp;
  }

  audp->nextsocket = nullptr;
}


void TIp4Handler::Run()
{
}

bool TIp4Handler::HandleRxPacket(TPacketMem * pmem)  // return true, if the packet is handled
{
  rxpkt = pmem;
  rxeh = PEthernetHeader(&pmem->data[0]);

  uint16_t etype = __REV16(rxeh->ethertype);

  if (0x0806 == etype) // ARP ?
  {
    return HandleArp();
  }
  else if (0x800 == etype) // IP
  {
    rxiph = PIp4Header(rxeh + 1);

    if (1 == rxiph->protocol) // ICMP ?
    {
      return HandleIcmp();
    }
    else if (17 == rxiph->protocol) // UDP ?
    {
      return HandleUdp();
    }
  }

  return false;
}

bool TIp4Handler::HandleArp()
{
  uint32_t  n;
  TPacketMem * pmem;

  PArpHeader parp = PArpHeader(rxeh + 1);

  TRACE("ARP request for %u.%u.%u.%u\r\n", parp->tpa[0], parp->tpa[1], parp->tpa[2], parp->tpa[3] );

  if (*(uint32_t *)&(parp->tpa) == ipaddress.u32)
  {
    TRACE("Answering ARP...\r\n");

    // prepare the answer

    pmem = adapter->AllocateTxPacket();
    if (!pmem)
    {
      return false;
    }

    pmem->datalen = sizeof(TEthernetHeader) + sizeof(TArpHeader);
    memcpy(&pmem->data[0], &rxpkt->data[0], pmem->datalen);

    PEthernetHeader txeh = PEthernetHeader(&pmem->data[0]);
    parp = PArpHeader(txeh + 1);

    parp->oper = 0x0200; // ARP Reply (byte swapped)

    // fill the ETH addresses
    for (n = 0; n < 6; ++n)
    {
      txeh->dest_mac[n] = txeh->src_mac[n];
      parp->tha[n] = parp->sha[n];
      txeh->src_mac[n] = adapter->peth->mac_address[n];
      parp->sha[n] = adapter->peth->mac_address[n];
    }

    // fill the IP addresses
    for (n = 0; n < 4; ++n)
    {
      parp->tpa[n] = parp->spa[n];
      parp->spa[n] = ipaddress.u8[n];
    }

    // send the packet
    adapter->SendTxPacket(pmem);  // the tx packet will be released automatically
  }

  return true;
}

bool TIp4Handler::HandleIcmp()
{
  uint32_t  n;
  TPacketMem * pmem;

  PIcmpHeader rxich = PIcmpHeader(rxiph + 1);
  if (8 == rxich->type) // echo request ?
  {
    TRACE("Echo request detected.\r\n");

    // prepare the answer

    pmem = adapter->AllocateTxPacket();
    if (!pmem)
    {
      return false;
    }

    pmem->datalen = rxpkt->datalen;
    memcpy(&pmem->data[0], &rxpkt->data[0], pmem->datalen);

    PEthernetHeader txeh = PEthernetHeader(&pmem->data[0]);
    PIp4Header  txiph = PIp4Header(txeh + 1);
    PIcmpHeader txich = PIcmpHeader(txiph + 1);

    // fill the ETH addresses
    for (n = 0; n < 6; ++n)
    {
      txeh->dest_mac[n] = txeh->src_mac[n];
      txeh->src_mac[n] = adapter->peth->mac_address[n];
    }

    // fill the IP addresses
    for (n = 0; n < 4; ++n)
    {
      txiph->dstaddr[n] = txiph->srcaddr[n];
      txiph->srcaddr[n] = ipaddress.u8[n];
    }

    txich->type = 0; // ICMP ECHO reply
    txich->code = 0;
    txich->cksum = 0;

    // the ICMP message can contain arbitrary length data, we need to calculate its size first
    uint16_t icmp_len = __REV16(txiph->len) - sizeof(TIp4Header);
    txich->cksum = __REV16(calc_icmp_checksum(txich, icmp_len));

    // send the packet
    adapter->SendTxPacket(pmem);  // the tx packet will be released automatically
  }
  else
  {
    return false;
  }

  return true;
}

bool TIp4Handler::HandleUdp()
{
  return true;
}

