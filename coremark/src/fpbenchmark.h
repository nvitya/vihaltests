// file:     fpbenchmark.h
// brief:    Simple FP Benchmark, testing multiplication and division only
// created:  2024-07-17
// authors:  nvitya

#ifndef SRC_FPBENCHMARK_H_
#define SRC_FPBENCHMARK_H_

float fpbench_f32(float f1, float f2, int iterations);
double fpbench_f64(double f1, double f2, int iterations);

void fpbenchmark_run(int iterations);

#endif /* SRC_FPBENCHMARK_H_ */
