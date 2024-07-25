#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include <stdint.h>

#define BLOCK_SIZE 2560
#define SAMPLE_RATE 256

// 함수 프로토타입
void init_fir_filter();
void clean_fir_filter();
void apply_fir_filter(float *input_signal, float *output_signal,
                      int signal_length);

#endif  // FIR_FILTER_H
