#include "sfft.h"

#include <complex.h>
#include <math.h>

#include "esp_dsp.h"

// #define ENABLE_LOGGING

int init_sfft() {
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE("dsp init", "Failed to initialize FFT. Error = %i", ret);
        return ret;
    }

    return 0;
}

void stft(float *data, float complex *sfft_result, int data_length,
          const int n_fft, const int win_length, const int hop_length,
          const int n_frames) {
    float *window = (float *)malloc(win_length * sizeof(float));
    if (window == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;  // 메모리 할당 실패 시 함수 종료
    }

    dsps_wind_hann_f32(window, win_length);

    for (int frame = 0; frame < n_frames; frame++) {
        int start = frame * hop_length;

        // Apply window and prepare for FFT
        for (int i = 0; i < win_length; i++) {
            int idx = start + i;
            if (idx < data_length) {
                sfft_result[frame * n_fft + i] = data[idx] * window[i];
            } else {
                sfft_result[frame * n_fft + i] = 0;
            }
        }
        // Zero padding if necessary
        for (int i = win_length; i < n_fft; i++) {
            sfft_result[frame * n_fft + i] = 0;
        }

        // Perform FFT
        dsps_fft2r_fc32((float *)&sfft_result[frame * n_fft], n_fft);
        dsps_bit_rev_fc32((float *)&sfft_result[frame * n_fft], n_fft);

#ifdef ENABLE_LOGGING
        // Print STFT result for the current frame
        ESP_LOGI("STFT", "Frame %d:", frame);
        for (int i = 0; i < n_fft / 2; i++) {
            float magnitude = cabsf(result[frame * n_fft + i]);
            ESP_LOGI("STFT", "  Bin %d: %f", i, magnitude);
        }
#endif
    }

    free(window);
}

void print_log_spectrogram(int rows, int cols, float *log_spectrogram) {
    for (int j = 4; j < 50; j++) {
        printf("raw%d: ", j);
        for (int i = 0; i < rows; i++) {
            // 1차원 배열처럼 접근하기 위해 인덱스를 계산
            printf("%.1f", *(log_spectrogram + i * cols + j));
            if (i < rows - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

void compute_log_spectrogram(float complex *sfft_result, float *log_spectrogram,
                             const int n_frames, const int n_fft) {
    for (int frame = 0; frame < n_frames; frame++) {
        for (int i = 0; i < n_fft / 2; i++) {
            float magnitude = cabsf(sfft_result[frame * n_fft + i]);
            log_spectrogram[frame * (n_fft / 2) + i] =
                100 * log10f(magnitude * magnitude);
        }
    }
}
