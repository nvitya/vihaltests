/*
 *  file:     traces.h (uart)
 *  brief:    Trace message redirection to UART
 *  version:  1.00
 *  date:     2021-10-02
 *  authors:  nvitya
*/

#ifndef __Traces__h
#define __Traces__h

#include "hwuart.h"

extern THwUart   conuart;  // console uart

#define TRACES

#define CC_NRM  "\x1B[0m"
#define CC_RED  "\x1B[31m"
#define CC_GRN  "\x1B[32m"
#define CC_YEL  "\x1B[33m"
#define CC_BLU  "\x1B[34m"
#define CC_MAG  "\x1B[35m"
#define CC_CYN  "\x1B[36m"
#define CC_WHT  "\x1B[37m"

#ifdef TRACES
  // add trace calls
  #define TRACE(...)  		{ conuart.printf( __VA_ARGS__ ); }
#else
  // ignore traces
  #define TRACE(...)
#endif

#ifdef LTRACES
 #define LTRACE(...)  TRACE( __VA_ARGS__ )
#else
 #define LTRACE(...)
#endif

#undef LTRACES

#endif //!defined(Traces__h)

//--- End Of file --------------------------------------------------------------
