#include <math.h>
#include <stdio.h>

#include "esp_dsp.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iir_filter.h"
#include "sfft.h"
#include "signal_processing.h"

static const char *TAG = "STFT_Example";

// Input signal
__attribute__((aligned(16))) float filtered_signal[N_SAMPLES];

void generate_sine_wave(float *signal, int length, float sample_rate) {
    for (int i = 0; i < length; i++) {
        signal[i] = sinf(2 * M_PI * 10.0f * i / sample_rate) +
                    sinf(2 * M_PI * 5.0f * i / sample_rate) +
                    sinf(2 * M_PI * 15.0f * i / sample_rate);
    }
}

void app_main() {
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize FFT. Error = %i", ret);
        return;
    }

    // Generate a sample input signal (e.g., sine wave)
    generate_sine_wave(input_signal, N_SAMPLES, N_FFT);

    // 신호 전처리
    preprocess_signal(input_signal, N_SAMPLES);

    apply_iir_filter(input_signal, filtered_signal, N_SAMPLES);

    // Perform STFT
    stft(filtered_signal, N_SAMPLES, N_FFT, WIN_LENGTH, HOP_LENGTH,
         stft_result);

    // Compute log spectrogram
    float log_spectrogram[N_FRAME * (N_FFT / 2)];
    compute_log_spectrogram(stft_result, log_spectrogram);

    ESP_LOGI(TAG, "STFT completed");
}
