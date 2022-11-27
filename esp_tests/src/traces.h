/*
 *  file:     traces.h
 *  brief:    Traces to UART
 *  date:     2021-10-02
 *  authors:  nvitya
*/

#ifndef __Traces__h
#define __Traces__h

#include "hwuart.h"

extern THwUart   conuart;  // console uart

#define TRACES

#ifdef TRACES
  // add trace calls
  #define TRACE(...)      { conuart.printf( __VA_ARGS__ ); }
#else
  // ignore traces
  #define TRACE(...)
#endif

#define TRACE_FLUSH(...)

#ifdef LTRACES
 #define LTRACE(...)  TRACE( __VA_ARGS__ )
#else
 #define LTRACE(...)
#endif

#undef LTRACES

#endif //!defined(Traces__h)
