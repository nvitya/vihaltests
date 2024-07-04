/*
 *  file:     traces.h (linux)
 *  brief:    Trace message redirection to stdout
 *  date:     2024-07-03
 *  authors:  nvitya
*/

#ifndef __Traces__h
#define __Traces__h

#include "stdio.h"

#define TRACES

#ifdef TRACES
  // add trace calls
  #define TRACE(...)  { printf( __VA_ARGS__ ); }
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

//--- End Of file --------------------------------------------------------------
