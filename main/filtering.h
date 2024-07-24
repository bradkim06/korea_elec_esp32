#ifndef FILTERING_H
#define FILTERING_H

#include "dsps_fir.h"

// Filter parameters
#define SAMPLE_RATE 256
#define SIGNAL_LENGTH 2560
#define BLOCK_SIZE 2560

#define NUMTAPS 31
#define IIR_FILTER_ORDER 2

typedef struct {
    float *output_data;
    float *fir_coeffs;
    float fir_state[NUMTAPS];
    fir_f32_t fir;
} EEGFilter;

void eeg_filter_init();
void eeg_filter_cleanup();
void eeg_filtering(float *input, float *result);

#endif  // FILTERING_H
