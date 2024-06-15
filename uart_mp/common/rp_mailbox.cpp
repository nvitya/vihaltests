/*
 * rp_mailbox.cpp
 *
 *  Created on: Jun 15, 2024
 *      Author: vitya
 */

#include <rp_mailbox.h>

void TRpMailBox::Init(uint8_t acoreid)
{
  coreid = acoreid;

  regs     = (TRpMailBoxRegs *)(SIO_BASE + RP_FIFO_REGS_OFFSET);
  //regs_set = (TRpMailBoxRegs *)(SIO_BASE + RP_FIFO_REGS_OFFSET + 0x2000);
  //regs_clr = (TRpMailBoxRegs *)(SIO_BASE + RP_FIFO_REGS_OFFSET + 0x3000);
}

bool TRpMailBox::TrySend(uint32_t adata)
{
  if (regs->ST & RP_MBX_FIFO_ST_RDY)
  {
    regs->WR = adata;
    __SEV();  // send the event to the other core
    return true;
  }
  else
  {
    return false;
  }
}

void TRpMailBox::Send(uint32_t adata)
{
  while (!TrySend(adata))
  {
    // wait
  }
}

uint32_t TRpMailBox::Recv()
{
  uint32_t r;
  while (!TryRecv(&r))
  {
    // wait
  }
  return r;
}


bool TRpMailBox::TryRecv(uint32_t *rdata)
{
  if (regs->ST & RP_MBX_FIFO_ST_VLD)
  {
    *rdata = regs->RD;
    return true;
  }
  else
  {
    return false;
  }
}

void multicore_launch_core1_raw(void (*entry)(void), uint32_t * sp, uint32_t vector_table)
{
  // Allow for the fact that the caller may have already enabled the FIFO IRQ for their
  // own purposes (expecting FIFO content after core 1 is launched). We must disable
  // the IRQ during the handshake, then restore afterwards.

  ///bool enabled = irq_is_enabled(SIO_IRQ_PROC0);
  ///irq_set_enabled(SIO_IRQ_PROC0, false);

  // Values to be sent in order over the FIFO from core 0 to core 1
  //

  ///irq_set_enabled(SIO_IRQ_PROC0, enabled);
}


void TRpMailBox::StartSecodaryCore(uint32_t aentry, uint32_t astack, uint32_t avectors)
{
  // vector_table is value for VTOR register
  // sp is initial stack pointer (SP)
  // entry is the initial program counter (PC) (don't forget to set the thumb bit!)
  const uint32_t cmd_sequence[] = { 0, 0, 1, avectors, astack, aentry };

  uint32_t response = 0;
  uint32_t seq = 0;
  do
  {
    uint32_t cmd = cmd_sequence[seq];
    // Always drain the READ FIFO (from core 1) before sending a 0
    if (!cmd)
    {
      while (TryRecv(&response)) { }
      //multicore_fifo_drain();

      // Execute a SEV as core 1 may be waiting for FIFO space via WFE
      __SEV();
    }

    Send(cmd);
    response = Recv();

    // Move to next state on correct response (echo-d value) otherwise start over
    if (cmd != response)
    {
      seq = 0; // start over
    }
    else
    {
      ++seq;
    }
  }
  while (seq < (sizeof(cmd_sequence) >> 2));
}

