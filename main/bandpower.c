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

static void calc_bands_mean();
static void print_band_means();
static float mean(float *array, int size);
static int is_drowsy(float power_bands[5][N_FRAMES], int num_frames,
                     float ratio_threshold, float *last_band);

// Input signal
__attribute__((aligned(16))) float log_spectrogram[N_FRAMES * (N_FFT / 2)];
// STFT result
__attribute__((aligned(16))) float complex
    sfft_result[N_FFT * ((N_SAMPLES - WIN_LENGTH) / HOP_LENGTH + 1)];
// 예시 데이터 크기 (num_bands * n_frames)
__attribute__((aligned(16))) float band_means[5][N_FRAMES];

typedef struct {
    int *freqBin;
    int size;
} Band;

Band bands[5];  // Declare the array globally

Band createBand(int start, int count) {
    Band b;
    b.size = count;
    b.freqBin = (int *)malloc(count * sizeof(int));
    for (int i = 0; i < count; ++i) {
        b.freqBin[i] = start + i;
    }
    return b;
}

int init_bandpower() {
    init_sfft();

    // Initialize each band using createBand
    bands[0] = createBand(1, 3);    // Delta
    bands[1] = createBand(4, 4);    // Theta
    bands[2] = createBand(8, 6);    // Alpha
    bands[3] = createBand(14, 16);  // Beta
    bands[4] = createBand(30, 20);  // Gamma

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

    calc_bands_mean();

    const int num_bands = sizeof(bands) / sizeof(bands[0]);
    for (int band = 0; band < num_bands; band++) {
        apply_kalman_filter(band_means[band], N_FRAMES);
    }

    // print_band_means();

    float ratio_threshold = 2.7;
    float last_band[4] = {0, 0, 0, 0};

    init_bandpower();  // 밴드 초기화

    int drowsy = is_drowsy(band_means, N_FRAMES, ratio_threshold, last_band);

    printf("Final drowsy state: %d\n", drowsy);
    printf("Last band values: Delta: %f, Theta: %f, Alpha: %f, Beta: %f\n",
           last_band[0], last_band[1], last_band[2], last_band[3]);

    return 0;
}

static void calc_bands_mean() {
    const int num_bands = sizeof(bands) / sizeof(bands[0]);

    for (int frame = 0; frame < N_FRAMES; frame++) {
        for (int band = 0; band < num_bands; band++) {
            float sum = 0.0;
            for (int j = 0; j < bands[band].size; j++) {
                int index = bands[band].freqBin[j];
                sum += log_spectrogram[frame * (N_FFT / 2) + index];
            }
            band_means[band][frame] = sum / bands[band].size;
        }
    }
}

static void print_band_means() {
    const int num_bands = sizeof(bands) / sizeof(bands[0]);

    printf("\nBand Means:\n");
    for (int band = 0; band < num_bands; band++) {
        printf("Band %d: [", band);
        for (int frame = 0; frame < N_FRAMES; frame++) {
            printf("%f", band_means[band][frame]);
            if (frame < N_FRAMES - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

static float mean(float *array, int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum / size;
}

static int is_drowsy(float power_bands[5][N_FRAMES], int num_frames,
                     float ratio_threshold, float *last_band) {
    const int window_size = 6;
    int *drowsy_states =
        (int *)malloc((num_frames - window_size + 1) * sizeof(int));
    int drowsy_state_count = 0;

    for (int i = 0; i <= num_frames - window_size; i++) {
        float cur_delta = mean(power_bands[0] + i, window_size);
        float cur_theta = mean(power_bands[1] + i, window_size);
        float cur_alpha = mean(power_bands[2] + i, window_size);
        float cur_beta = mean(power_bands[3] + i, window_size);

        float ratio = (cur_theta + cur_alpha) / cur_beta;
        if (cur_beta == 0) {
            ratio = INFINITY;
        }

        if (ratio > ratio_threshold) {
            drowsy_states[i] = 1;
            drowsy_state_count++;
        } else {
            drowsy_states[i] = 0;
        }

        // 마지막 데이터 저장
        if (i + window_size >= num_frames) {
            last_band[0] = cur_delta;
            last_band[1] = cur_theta;
            last_band[2] = cur_alpha;
            last_band[3] = cur_beta;
        }
    }

    // 졸음 상태 비율 계산
    float drowsy_ratio =
        (float)drowsy_state_count / (num_frames - window_size + 1);

    // 졸음 여부 판단 (예: 50% 이상이 졸음 상태일 경우)
    int final_drowsy_state = (drowsy_ratio >= 0.5) ? 1 : 0;

    free(drowsy_states);
    return final_drowsy_state;
}
