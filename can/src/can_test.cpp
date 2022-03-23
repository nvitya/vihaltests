/*
 * can_test.cpp
 *
 *  Created on: Mar 23, 2022
 *      Author: vitya
 */

#include "hwcan.h"
#include "board_pins.h"
#include "can_test.h"
#include "traces.h"

// allocate the CAN SW buffers:
TCanMsg   can_rxbuf[16];
TCanMsg   can_txbuf[16];

TCanMsg   msg;

unsigned  last_send_time = 0;
unsigned  msgcounter = 0;

void can_test_init()
{
  can.Init(can_devnum, &can_rxbuf[0], sizeof(can_rxbuf) / sizeof(TCanMsg), &can_txbuf[0], sizeof(can_txbuf) / sizeof(TCanMsg));

  can.speed = 1000000;
  can.raw_timestamp = true; // timestamps are the original 16-bit CAN bit clock timestamps
  can.AcceptAdd(0x000, 0x000); // accept all messages
  can.Enable(); // start the CAN (reception)
}

void can_test_run()
{
  unsigned t = CLOCKCNT;

  // if no interrupt is used then these must be called regularly
  can.HandleTx();  // sends the buffered messages from SW buffer
  // can.HandleRx(); // moves the Rx message to the SW buffer. This is included in can.TryRecvMessage()

  if (can.TryRecvMessage(&msg))
  {
    TRACE("CAN MSG(TS=%04X): COBID=%03X, LEN=%i, DATA=", msg.timestamp, msg.cobid, msg.len);
    for (int i = 0; i < msg.len; ++i)
    {
      TRACE(" %02X", msg.data[i]);
    }
    TRACE("\r\n");
  }

  can.UpdateErrorCounters(); // to count ack, stuff, form and crc errors

  if (pin_led_count > 2)
  {
    // turn on the led[2] on errors
    if (can.IsWarning() || can.IsBusOff())
    {
      pin_led[2].Set1();
    }
    else
    {
      pin_led[2].Set0();
    }
  }

  // send a message every second
  if (t - last_send_time > SystemCoreClock)
  {
    ++msgcounter;

    pin_led[0].SetTo(msgcounter & 1);

    // send out two test messages
    msg.cobid = 0x203;
    msg.len = 8;
    *(unsigned *)&msg.data[0] = msgcounter;
    msg.data[4] = 0x15;
    msg.data[5] = 0x16;
    msg.data[6] = 0x17;
    msg.data[7] = 0x18;
    can.StartSendMessage(&msg);

    msg.cobid = 0x303;
    msg.len = 8;
    *(unsigned *)&msg.data[0] = msgcounter;
    msg.data[4] = 0x25;
    msg.data[5] = 0x26;
    msg.data[6] = 0x27;
    msg.data[7] = 0x28;
    can.StartSendMessage(&msg);

    last_send_time = t;
  }

}
