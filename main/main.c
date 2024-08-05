#include <math.h>
#include <stdio.h>

#include "esp_dsp.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sfft.h"

static const char *TAG = "STFT_Example";

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

    // Perform STFT
    stft(input_signal, N_SAMPLES, N_FFT, WIN_LENGTH, HOP_LENGTH, stft_result);

    // Display results (magnitude of STFT)
    int n_frames = 1 + (N_SAMPLES - WIN_LENGTH) / HOP_LENGTH;
    for (int frame = 0; frame < n_frames; frame++) {
        ESP_LOGI(TAG, "Frame %d:", frame);
        for (int i = 0; i < N_FFT / 2; i++) {
            float magnitude = cabsf(stft_result[frame * N_FFT + i]);
            ESP_LOGI(TAG, "  Bin %d: %f", i, magnitude);
        }
        vTaskDelay(10);
    }

    ESP_LOGI(TAG, "STFT completed");
}
