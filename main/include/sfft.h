#ifndef SFFT_H
#define SFFT_H

#include <complex.h>

// Define constants
#define N_SAMPLES 256
#define N_FFT 256
#define WIN_LENGTH N_FFT
#define HOP_LENGTH 128

// Input signal
extern float input_signal[N_SAMPLES];
// Window coefficients
extern float window[WIN_LENGTH];
// STFT result
extern float complex
    stft_result[N_FFT * ((N_SAMPLES - WIN_LENGTH) / HOP_LENGTH + 1)];

// Function declarations
void stft(float *data, int data_length, int n_fft, int win_length,
          int hop_length, float complex *result);

#endif  // SFFT_H
