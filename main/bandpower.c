#include "bandpower.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_dsp.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iir_filter.h"
#include "kal_filter.h"
#include "sfft.h"
#include "signal_processing.h"

typedef struct {
    int *indices;
    int count;
} Band;

// Input signal
__attribute__((aligned(16))) float log_spectrogram[N_FRAMES * (N_FFT / 2)];
// STFT result
__attribute__((aligned(16))) float complex
    sfft_result[N_FFT * ((N_SAMPLES - WIN_LENGTH) / HOP_LENGTH + 1)];

int init_bandpower() {
    init_sfft();

    return 0;
}

int bandpower(float *input_signal) {
    // 신호 전처리
    preprocess_signal(input_signal, N_SAMPLES);

    // 필터링
    apply_iir_filter(input_signal, N_SAMPLES);

    // Perform STFT
    stft(input_signal, sfft_result, N_SAMPLES, N_FFT, WIN_LENGTH, HOP_LENGTH,
         N_FRAMES);

    compute_log_spectrogram(sfft_result, log_spectrogram, N_FRAMES, N_FFT);

    apply_kalman_filter(log_spectrogram, N_FRAMES * (N_FFT / 2));

    print_log_spectrogram(N_FRAMES, N_FFT / 2, log_spectrogram);

    // 주파수 대역 인덱스
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
