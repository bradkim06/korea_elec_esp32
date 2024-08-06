#ifndef SFFT_H
#define SFFT_H

#include <complex.h>

// Function declarations
int init_sfft();
void stft(float *data, float complex *sfft_result, int data_length,
          const int n_fft, const int win_length, const int hop_length,
          const int n_frames);
void compute_log_spectrogram(float complex *sfft_result, float *log_spectrogram,
                             const int n_frames, const int n_fft);
void print_log_spectrogram(int rows, int cols, float *log_spectrogram);

#endif  // SFFT_H
