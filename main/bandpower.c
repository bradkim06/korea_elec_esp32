
#include "bandpower.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "eeg_signal.h"
#include "esp_dsp.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iir_filter.h"
#include "sfft.h"
#include "signal_processing.h"

typedef struct {
    int *indices;
    int count;
} Band;

static const char *TAG = "STFT_Example";

// Input signal
__attribute__((aligned(16))) float filtered_signal[N_SAMPLES];

void generate_sine_wave(float *signal, int length, float sample_rate) {
    for (int i = 0; i < length; i++) {
        // signal[i] = sinf(2 * M_PI * 5.0f * i / sample_rate) +
        //             sinf(2 * M_PI * 100.0f * i / sample_rate) +
        //             sinf(2 * M_PI * 0.1f * i / sample_rate);
        signal[i] = eeg_signal[i];
    }
}

int bandpower() {
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize FFT. Error = %i", ret);
        return ret;
    }

    // Generate a sample input signal (e.g., sine wave)
    generate_sine_wave(input_signal, N_SAMPLES, N_FFT);

    // 신호 전처리
    preprocess_signal(input_signal, N_SAMPLES);

    apply_iir_filter(input_signal, filtered_signal, N_SAMPLES);

    // for (int i = 0; i < 512; i++) {
    //     printf("filtered %d: %f %f\n", i, input_signal[i],
    //     filtered_signal[i]);
    // }

    // Perform STFT
    stft(filtered_signal, N_SAMPLES, N_FFT, WIN_LENGTH, HOP_LENGTH,
         stft_result);

    // Compute log spectrogram
    float log_spectrogram[N_FRAME * (N_FFT / 2)];
    compute_log_spectrogram(stft_result, log_spectrogram);

    // 주파수 대역 인덱스를 강제로 설정
    Band bands[] = {
        {(int[]){1, 2, 3}, 3},               // Delta
        {(int[]){4, 5, 6, 7}, 4},            // Theta
        {(int[]){8, 9, 10, 11, 12, 13}, 6},  // Alpha
        {(int[]){14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                 29},
         16},  // Beta
        {(int[]){30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                 40, 41, 42, 43, 44, 45, 46, 47, 48, 49},
         20}  // Gamma
    };
    int band_count = sizeof(bands) / sizeof(bands[0]);

    // 결과 출력 (예시)
    const char *band_names[] = {"Delta", "Theta", "Alpha", "Beta", "Gamma"};
    for (int b = 0; b < band_count; b++) {
        printf("%s band indices: ", band_names[b]);
        for (int i = 0; i < bands[b].count; i++) {
            printf("%d ", bands[b].indices[i]);
        }
        printf("\n");
    }

    return 0;
}
