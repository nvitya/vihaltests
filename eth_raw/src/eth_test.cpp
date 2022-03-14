/*
 * eth_test.cpp
 *
 *  Created on: Mar 14, 2022
 *      Author: vitya
 */

#include "string.h"
#include "board_pins.h"
#include "traces.h"

uint8_t   my_ip_address[4] = {192, 168, 2, 10};

__attribute__((aligned(32)))
uint8_t   eth_rx_desc_mem[sizeof(HW_ETH_DMA_DESC) * ETH_RX_PACKETS];

__attribute__((aligned(32)))
uint8_t   eth_tx_desc_mem[sizeof(HW_ETH_DMA_DESC) * ETH_TX_PACKETS];

uint8_t   eth_rx_packet_mem[HWETH_MAX_PACKET_SIZE * ETH_RX_PACKETS]   __attribute__((aligned(16)));

unsigned last_recv_time = 0;

void eth_test_init()
{
  //eth.promiscuous_mode = true;
  eth.promiscuous_mode = false;
  eth.hw_ip_checksum = false; //true;
  if (!eth.Init(&eth_rx_desc_mem, ETH_RX_PACKETS, &eth_tx_desc_mem, ETH_TX_PACKETS))
  {
    TRACE("ETH INIT FAILED !!!\r\n");
  }
  else
  {
    TRACE("ETH init ok.\r\n");
  }

  // there is no valid rx buffer yet!

  for (unsigned n = 0; n < ETH_RX_PACKETS; ++n)
  {
    eth.AssignRxBuf(n, &eth_rx_packet_mem[HWETH_MAX_PACKET_SIZE * n], HWETH_MAX_PACKET_SIZE);
  }

  eth.Start();

  last_recv_time = CLOCKCNT;
}

typedef struct TEthernetHeader
{
  uint8_t   dest_mac[6];  /**< Destination node */
  uint8_t   src_mac[6];   /**< Source node */
  uint16_t  ethertype;    /**< Protocol or length */
//
} __attribute__((packed))  TEthernetHeader, * PEthernetHeader;

typedef struct TArpHeader
{
  uint16_t  hrd;    /**< Format of hardware address */
  uint16_t  pro;    /**< Format of protocol address */
  uint8_t   hln;    /**< Length of hardware address */
  uint8_t   pln;    /**< Length of protocol address */
  uint16_t  op;     /**< Operation */
  uint8_t   sha[6]; /**< Sender hardware address */
  uint8_t   spa[4]; /**< Sender protocol address */
  uint8_t   tha[6]; /**< Target hardware address */
  uint8_t   tpa[4]; /**< Target protocol address */
//
} __attribute__((packed))  TArpHeader, * PArpHeader;

typedef struct TIpHeader
{
  uint8_t   hl_v;   /**< Header length and version */
  uint8_t   tos;    /**< Type of service */
  uint16_t  len;    /**< Total length */
  uint16_t  id;     /**< Identification */
  uint16_t  off;    /**< Fragment offset field */
  uint8_t   ttl;    /**< Time to live */
  uint8_t   p;      /**< Protocol */
  uint16_t  sum;    /**< Checksum */
  uint8_t   src[4]; /**< Source IP address */
  uint8_t   dst[4]; /**< Destination IP address */
//
} __attribute__((packed))  TIpHeader, * PIpHeader;

typedef struct icmp_echo_header
{
  uint8_t  type;   /**< Type of message */
  uint8_t  code;   /**< Type subcode */
  uint16_t cksum;  /**< 1's complement cksum of struct */
  uint16_t id;     /**< Identifier */
  uint16_t seq;    /**< Sequence number */
//
} __attribute__((packed))  TIcmpHeader, * PIcmpHeader;

uint8_t pbuf[1536];

void answer_arp(uint8_t * pdata)
{
  uint32_t  n;
  PEthernetHeader peth;
  PArpHeader parp;
  uint32_t idx;
  peth = PEthernetHeader(pdata);
  parp = PArpHeader(pdata + sizeof(TEthernetHeader));

  TRACE("ARP request for %u.%u.%u.%u\r\n", parp->tpa[0], parp->tpa[1], parp->tpa[2], parp->tpa[3] );

  if (*(uint32_t *)&(parp->tpa) == *(uint32_t *)&(my_ip_address))
  {
    TRACE("Answering ARP...\r\n");

    // prepare the answer
    memcpy(&pbuf[0], pdata, sizeof(TEthernetHeader) + sizeof(TArpHeader));

    peth = PEthernetHeader(&pbuf[0]);
    parp = PArpHeader(peth + 1);

    parp->op = 0x0200; // ARP Reply (byte swapped)

    // fill the ETH addresses
    for (n = 0; n < 6; ++n)
    {
      peth->dest_mac[n] = peth->src_mac[n];
      parp->tha[n] = parp->sha[n];
      peth->src_mac[n] = eth.mac_address[n];
      parp->sha[n] = eth.mac_address[n];
    }

    // fill the IP addresses
    for (n = 0; n < 4; ++n)
    {
      parp->tpa[n] = parp->spa[n];
      parp->spa[n] = my_ip_address[n];
    }

    // send the packed
    if (!eth.TrySend(&idx, &pbuf[0], sizeof(TEthernetHeader) + sizeof(TArpHeader)))
    {
      TRACE("Packet send failed.\r\n");
    }
  }
}

uint16_t calc_icmp_checksum(void * pdata, uint32_t datalen)
{
  //((uint8_t *)&pdata)[datalen] = 0; // for odd size handling

  uint32_t n;
  uint32_t clen = ((datalen + 1) >> 1);
  uint32_t sum = 0;
  uint16_t * pd16 = (uint16_t *)pdata;

  for (n = 0; n < clen; ++n)
  {
    sum += __REV16(*pd16++);
  }

  sum = (sum & 0xffff) + (sum >> 16);


  return (uint16_t) (~sum);
}


void answer_ip(uint8_t * pdata, uint16_t datalen)
{
  uint32_t n;
  PEthernetHeader  peth;
  PIpHeader        pih;
  uint32_t         idx;
  peth = PEthernetHeader(pdata);
  pih  = PIpHeader(pdata + sizeof(TEthernetHeader));

  uint8_t prot = pih->p;

  TRACE("IP protocol = %u\r\n", prot);

  if (prot == 1) // ICMP
  {
    PIcmpHeader pich = PIcmpHeader(pih + 1);
    if (pich->type == 8) // echo request ?
    {
      TRACE("Echo request detected.\r\n");

      // prepare the answer
      memcpy(&pbuf[0], pdata, datalen);

      peth = PEthernetHeader(&pbuf[0]);
      pih = PIpHeader(peth + 1);
      pich = PIcmpHeader(pih + 1);

      // fill the ETH addresses
      for (n = 0; n < 6; ++n)
      {
        peth->dest_mac[n] = peth->src_mac[n];
        peth->src_mac[n] = eth.mac_address[n];
      }

      // fill the IP addresses
      for (n = 0; n < 4; ++n)
      {
        pih->dst[n] = pih->src[n];
        pih->src[n] = my_ip_address[n];
      }

      pich->type = 0; // ICMP ECHO reply
      pich->code = 0;
      pich->cksum = 0;

      // the ICMP message can contain arbitrary length data, we need to calculate its size first
      uint16_t icmp_len = __REV16(pih->len) - sizeof(TIpHeader);
      pich->cksum = __REV16(calc_icmp_checksum(pich, icmp_len));

      // send the packed
      if (!eth.TrySend(&idx, &pbuf[0], datalen))
      {
        TRACE("ICMP Packet send failed.\r\n");
        return;
      }

      TRACE("ICMP response sent.\r\n");
    }
  }
}

void eth_test_run()
{
  uint32_t n;
  uint32_t idx;
  uint8_t * pdata;
  uint32_t  datalen;
  PEthernetHeader peh;

  eth.PhyStatusPoll(); // must be called regularly

  if (eth.TryRecv(&idx, (void * *)&pdata, &datalen))
  {
    unsigned t0 = CLOCKCNT;

    TRACE("+%u ms: ", (t0 - last_recv_time) / (SystemCoreClock / 1000));
    TRACE("Eth frame received, len = %u\r\n", datalen);

    last_recv_time = t0;

#if 1 // display contents
    for (n = 0; n < datalen; ++n)
    {
      if ((n > 0) && ((n & 15) == 0)) TRACE("\r\n");
      TRACE(" %02X", pdata[n]);
    }
    TRACE("\r\n");
#endif

    peh = PEthernetHeader(pdata);
    TRACE("  dst = %02X:%02X:%02X:%02X:%02X:%02X",
      peh->dest_mac[0],
      peh->dest_mac[1],
      peh->dest_mac[2],
      peh->dest_mac[3],
      peh->dest_mac[4],
      peh->dest_mac[5]
    );
    TRACE(", src = %02X:%02X:%02X:%02X:%02X:%02X",
      peh->src_mac[0],
      peh->src_mac[1],
      peh->src_mac[2],
      peh->src_mac[3],
      peh->src_mac[4],
      peh->src_mac[5]
    );

    uint16_t etype = __REV16(peh->ethertype);

    TRACE(", type = %04X\r\n", etype);

    if (etype == 0x0806) // ARP ?
    {
      answer_arp(pdata);
    }
    else if (etype == 0x800) // IP
    {
      answer_ip(pdata, datalen);
    }

    eth.ReleaseRxBuf(idx); // must be called !
  }
}
