/*
 * network.h
 *
 *  Created on: Mar 17, 2022
 *      Author: vitya
 */

#ifndef NETWORK_NETWORK_H_
#define NETWORK_NETWORK_H_

#include "stdint.h"

typedef struct
{
  uint8_t   dest_mac[6];  /**< Destination node */
  uint8_t   src_mac[6];   /**< Source node */
  uint16_t  ethertype;    /**< Protocol or length */
//
} TEthernetHeader, * PEthernetHeader;

typedef struct
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
} TArpHeader, * PArpHeader;

typedef struct
{
  uint8_t  type;   /**< Type of message */
  uint8_t  code;   /**< Type subcode */
  uint16_t cksum;  /**< 1's complement cksum of struct */
  uint16_t id;     /**< Identifier */
  uint16_t seq;    /**< Sequence number */
//
} TIcmpHeader, * PIcmpHeader;

uint16_t calc_icmp_checksum(void * pdata, uint32_t datalen);

#endif /* NETWORK_NETWORK_H_ */
