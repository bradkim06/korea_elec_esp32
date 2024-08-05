#include "sfft.h"

#include <complex.h>

#include "esp_dsp.h"

// Input signal
__attribute__((aligned(16))) float input_signal[N_SAMPLES];
// Window coefficients
__attribute__((aligned(16))) float window[WIN_LENGTH];
// STFT result
__attribute__((aligned(16))) float complex
    stft_result[N_FFT * ((N_SAMPLES - WIN_LENGTH) / HOP_LENGTH + 1)];

void stft(float *data, int data_length, int n_fft, int win_length,
          int hop_length, float complex *result) {
    int n_frames = 1 + (data_length - win_length) / hop_length;

    dsps_wind_hann_f32(window, win_length);

    for (int frame = 0; frame < n_frames; frame++) {
        int start = frame * hop_length;

        // Apply window and prepare for FFT
        for (int i = 0; i < win_length; i++) {
            int idx = start + i;
            if (idx < data_length) {
                result[frame * n_fft + i] = data[idx] * window[i];
            } else {
                result[frame * n_fft + i] = 0;
            }
        }
        // Zero padding if necessary
        for (int i = win_length; i < n_fft; i++) {
            result[frame * n_fft + i] = 0;
        }

        // Perform FFT
        dsps_fft2r_fc32((float *)&result[frame * n_fft], n_fft);
        dsps_bit_rev_fc32((float *)&result[frame * n_fft], n_fft);
    }
}
