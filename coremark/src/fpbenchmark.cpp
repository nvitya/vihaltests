/* License: public domain

The benchmark code is very simple, it is taken from this project

  https://github.com/jzawodn/arm-neon-vfp-test

*/

// file:     fpbenchmark.cpp
// brief:    Simple FP Benchmark, testing multiplication and division only
// created:  2024-07-17
// authors:  nvitya


#include "platform.h"
#include "hwuscounter.h"
#include "traces.h"

const double  default_f1            = 2.200002;
const double  default_f2            = 2.200001;
const int     default_million_iter  = 5;

float fpbench_f32(float f1, float f2, int iterations)
{
  float ans = 1.0;
  for(int i = 0; i < iterations; i++)
  {
    ans *= f1;
    ans /= f2;
  }
  return ans;
}

double fpbench_f64(double f1, double f2, int iterations)
{
  double ans = 1.0;
  for(int i = 0; i < iterations; i++)
  {
    ans *= f1;
    ans /= f2;
  }
  return ans;
}

void fpbenchmark_run(int iterations)
{
  uint32_t tstart, tend, elapsed_ms;

  double f1 = default_f1;
  double f2 = default_f2;
  double ans;

  TRACE("FP32 benchmark with F1=%f, F2=%f, iterations=%i:\r\n", f1, f2, iterations);
  tstart = uscounter.Get32();
  ans = fpbench_f32(f1, f2, iterations);
  tend = uscounter.Get32();
  elapsed_ms = (tend - tstart) / 1000;
  TRACE("  ans = %f, time = %.3f ms\r\n", ans, elapsed_ms);
  TRACE("  %d loop/msec\r\n", (int)(iterations/elapsed_ms));

  TRACE("FP64 benchmark with F1=%f, F2=%f, iterations=%i:\r\n", f1, f2, iterations);
  tstart = uscounter.Get32();
  ans = fpbench_f64(f1, f2, iterations);
  tend = uscounter.Get32();
  elapsed_ms = (tend - tstart) / 1000;
  TRACE("  ans = %f, time = %.3f ms\r\n", ans, elapsed_ms);
  TRACE("  %d loop/msec\r\n", (int)(iterations/elapsed_ms));
}
