#include "sfft.h"

#include <complex.h>
#include <math.h>

#include "esp_dsp.h"

// #define ENABLE_LOGGING

const int N_FRAME = 1 + (N_SAMPLES - WIN_LENGTH) / HOP_LENGTH;

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

#ifdef ENABLE_LOGGING
        // Print STFT result for the current frame
        ESP_LOGI("STFT", "Frame %d:", frame);
        for (int i = 0; i < n_fft / 2; i++) {
            float magnitude = cabsf(result[frame * n_fft + i]);
            ESP_LOGI("STFT", "  Bin %d: %f", i, magnitude);
        }
#endif
    }
}

void compute_log_spectrogram(float complex *stft_result,
                             float *log_spectrogram) {
    int n_frames = 1 + (N_SAMPLES - WIN_LENGTH) / HOP_LENGTH;

    for (int frame = 0; frame < n_frames; frame++) {
#ifdef ENABLE_LOGGING
        ESP_LOGI("Log Frame", "%d:", frame);
#endif
        for (int i = 0; i < N_FFT / 2; i++) {
            float magnitude = cabsf(stft_result[frame * N_FFT + i]);
            log_spectrogram[frame * (N_FFT / 2) + i] =
                100 * log10f(magnitude * magnitude);

#ifdef ENABLE_LOGGING
            // Print log spectrogram for the current frame
            ESP_LOGI("Log Spectrogram", "  Bin %d: %f", i,
                     log_spectrogram[frame * (N_FFT / 2) + i]);
#endif
        }
    }
}
