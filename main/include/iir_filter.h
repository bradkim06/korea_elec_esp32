#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#define BLOCK_SIZE 2560
#define SAMPLE_RATE 256

void apply_iir_filter(float* input_signal, float* output_signal,
                      int signal_length);
#endif
