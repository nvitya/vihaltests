/*
 *  file:     board.h (uart)
 *  brief:    Board / MCU definition
 *  version:  1.00
 *  date:     2021-10-02
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#if defined(BOARD_VRV1_103)

#define BOARD_NAME "VRV1_103"
#define MCU_VRV1_103
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_104)

#define BOARD_NAME "VRV1_104"
#define MCU_VRV1_104
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_241)

#define BOARD_NAME "VRV1_241"
#define MCU_VRV1_241
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_401)

#define BOARD_NAME "VRV1_401"
#define MCU_VRV1_401
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_441)

#define BOARD_NAME "VRV1_441"
#define MCU_VRV1_441
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_443)

#define BOARD_NAME "VRV1_443"
#define MCU_VRV1_443
#define MCU_FIXED_SPEED  100000000

#elif defined(BOARD_VRV1_543)

#define BOARD_NAME "VRV1_543"
#define MCU_VRV1_543
#define MCU_FIXED_SPEED  100000000

#else

  #error "unknown board"

#endif

#endif /* BOARD_H_ */
