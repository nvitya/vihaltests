/*
 *  file:     traces.h (uart)
 *  brief:    Dummy traces
 *  version:  1.00
 *  date:     2021-10-02
 *  authors:  nvitya
*/

#ifndef __Traces__h
#define __Traces__h

// ignore traces
#define TRACE(...)

#ifdef LTRACES
 #define LTRACE(...)  TRACE( __VA_ARGS__ )
#else
 #define LTRACE(...)
#endif

#undef LTRACES

#endif //!defined(Traces__h)
