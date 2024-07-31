#include <float.h>
#include <math.h>
#include <stdio.h>

#include "eeg_signal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iir_filter.h"

float filtered_signal[BLOCK_SIZE];

void filter_task(void* pvParameters) {
    // 예제 데이터 시그널
    for (int i = 0; i < BLOCK_SIZE; i++) {
        eeg_signal[i] = sin(2 * M_PI * 1.0 * i / SAMPLE_RATE) +
                        sin(2 * M_PI * 0.1 * i / SAMPLE_RATE) +
                        sin(2 * M_PI * 0.15 * i / SAMPLE_RATE) +
                        sin(2 * M_PI * 100.0 * i / SAMPLE_RATE) +
                        sin(2 * M_PI * 60.0 * i / SAMPLE_RATE);
    }

    apply_iir_filter(eeg_signal, filtered_signal, BLOCK_SIZE);

    // 결과 출력
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%f\n", filtered_signal[i]);
    }

    vTaskDelete(NULL);
}

void app_main() {
    xTaskCreate(filter_task, "filter_task", 4096, NULL, 5, NULL);
}
