/*
 *  file:     traces.h
 *  brief:    Trace message redirection to UART
 *  created:  2021-10-02
 *  authors:  nvitya
*/

#ifndef __Traces__h
#define __Traces__h

#include "hwuart.h"
#include "tracebuf.h"

extern THwUart   conuart;

extern void traces_init();

#define TRACES

#define TRACE_WAIT  0

#ifdef TRACES
  // add trace calls
  #define TRACE(...)  		  { tracebuf.printf( __VA_ARGS__ ); }
  #define TRACE_FLUSH(...)  { tracebuf.Flush(); }
#else
  // ignore traces
  #define TRACE(...)
  #define TRACE_FLUSH(...)
#endif

#ifdef LTRACES
 #define LTRACE(...)  TRACE( __VA_ARGS__ )
#else
 #define LTRACE(...)
#endif

#undef LTRACES

#endif //!defined(Traces__h)

//--- End Of file --------------------------------------------------------------
