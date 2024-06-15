/*
 * rp_mailbox.h
 *
 *  Created on: Jun 15, 2024
 *      Author: vitya
 */

#ifndef COMMON_RP_MAILBOX_H_
#define COMMON_RP_MAILBOX_H_

#include "platform.h"

#define RP_FIFO_REGS_OFFSET  0x050

typedef struct
{
  volatile uint32_t  ST;
  volatile uint32_t  WR;
  volatile uint32_t  RD;
//
} TRpMailBoxRegs;

#define RP_MBX_FIFO_ST_VLD  (1 << 0)
#define RP_MBX_FIFO_ST_RDY  (1 << 1)
#define RP_MBX_FIFO_ST_WOF  (1 << 2)
#define RP_MBX_FIFO_ST_ROE  (1 << 3)

class TRpMailBox
{
public:
  TRpMailBoxRegs *  regs     = nullptr;
  //TRpMailBoxRegs *  regs_set = nullptr;
  //TRpMailBoxRegs *  regs_clr = nullptr;
  uint8_t           coreid = 0;

  void              Init(uint8_t acoreid);
  void              Send(uint32_t adata);
  uint32_t          Recv();

  bool              TrySend(uint32_t adata);
  bool              TryRecv(uint32_t * rdata);

  void              StartSecodaryCore(uint32_t aentry, uint32_t astack, uint32_t avectors);
};

#endif /* COMMON_RP_MAILBOX_H_ */
