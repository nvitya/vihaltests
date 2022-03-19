/*
 * net_ip4.h
 *
 *  Created on: Mar 19, 2022
 *      Author: vitya
 */

#ifndef NETWORK_NET_IP4_H_
#define NETWORK_NET_IP4_H_

#include "stdint.h"
#include "network.h"
#include "hweth.h"

#define NETIF_PMEM_HEAD_SIZE  16
//#define NETIF_TX_PMEM_SIZE  (HWETH_MAX_PACKET_SIZE + NETIF_PMEM_HEAD_SIZE)
//#define NETIF_RX_PMEM_SIZE  (HWETH_MAX_PACKET_SIZE + NETIF_PMEM_HEAD_SIZE)

typedef struct TPacketMem
{
  uint8_t       idx;
  uint8_t       flags;
  uint16_t      datalen;
  TPacketMem *  next;    // this is reserved for the applications
  uint64_t      timestamp_ns;

  uint8_t       data[HWETH_MAX_PACKET_SIZE];
//
} TPacketMem, * PPacketMem;

typedef struct
{
  union
  {
    uint8_t   u8[4];
    uint32_t  u32;
  };

  inline void Set(int a0, int a1, int a2, int a3)
  {
    u8[0] = a0;
    u8[1] = a1;
    u8[2] = a2;
    u8[3] = a3;
  }
//
} TIp4Addr;

typedef struct
{
  uint8_t   hl_v;      /**< Header length and version */
  uint8_t   tos;       /**< Type of service */
  uint16_t  len;       /**< Total length */
  uint16_t  id;        /**< Identification */
  uint16_t  offset;    /**< Fragment offset field */
  uint8_t   ttl;       /**< Time to live */
  uint8_t   protocol;  /**< Protocol */
  uint16_t  csum;      /**< Checksum */
  uint8_t   srcaddr[4];  /**< Source IP address */
  uint8_t   dstaddr[4];  /**< Destination IP address */
//
} TIp4Header, * PIp4Header;


typedef struct
{
  uint16_t  sport;   // source port
  uint16_t  dport;   // destination port
  uint16_t  len;     // length
  uint16_t  csum;    // checksum
//
} TUdp4Header, * PUdp4Header;

typedef struct TArp4TableItem // 16 byte
{
  TIp4Addr  ipaddr;
  uint8_t   macaddr[6];
  uint8_t   _pad[2];
  uint32_t  timestamp_ms;
//
} TArp4TableItem, * PArp4TableItem;

class TProtocolHandler
{
public:

};

class TNetAdapter // could be the child of the THwEth too
{
protected:
  uint8_t           state = 0;

public: // settings

  uint8_t           max_rx_packets = 8;
  uint8_t           max_tx_packets = 8;
  uint8_t           max_arp_items = 8;

public:
  bool              initialized = false;
  THwEth *          peth = nullptr;
  uint8_t *         netmem = nullptr;
  unsigned          netmem_size = 0;
  unsigned          netmem_allocated = 0;

  TPacketMem *      first_tx_pmem = nullptr;
  TPacketMem *      syspkt = nullptr;

  uint8_t *         rx_desc_mem = nullptr;
  uint8_t *         tx_desc_mem = nullptr;

  uint8_t *         rx_pmem = nullptr;
  uint8_t *         tx_pmem = nullptr;

  void              Init(THwEth * aeth, void * anetmem, unsigned anetmemsize);

  void              Run(); // must be called regularly

  TPacketMem *      AllocateTxPacket();
  void              ReleaseTxPacket(TPacketMem * apmem);
};

class TIp4Handler : public TProtocolHandler
{
public:
  TIp4Addr          ipaddress;
  TIp4Addr          netmask;
  TIp4Addr          gwaddress;

  void              Init(TNetAdapter * aadapter);
};

class TUdp4Socket
{
public:
  TIp4Addr          destaddr;
  uint16_t          destport = 0;

  TIp4Addr          srcaddr;
  uint16_t          srcport = 0;

  TNetAdapter *     pnetif = nullptr;

  void Init(void * anif, uint16_t aport);

  int Send(void * adataptr, unsigned adatalen);
  int Receive(void * adataptr, unsigned adatalen);
};

uint16_t calc_udp4_checksum(TIp4Header * piph, uint16_t datalen);


#endif /* NETWORK_NET_IP4_H_ */
