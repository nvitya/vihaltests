
#ifndef BOARD_H_
#define BOARD_H_

#include "boards_builtin.h"

#ifdef MCUF_VRV100
  #define SELF_FLASHING 1
#endif

#ifndef SELF_FLASHING
  #define SELF_FLASHING 0
#endif

#endif /* BOARD_H_ */
