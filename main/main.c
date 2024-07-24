#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver/uart.h"
#include "eeg_signal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "signal_processing.h"

float result[SIGNAL_LENGTH] = {0.0f};

// 사용 예시
void app_main() {
    float mean_value = calculate_mean_abs(eeg_signal, SIGNAL_LENGTH);
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        eeg_signal[i] = eeg_signal[i] - mean_value;
    }

    remove_baseline_drift(eeg_signal, result, SIGNAL_LENGTH, 16, 0.5f);

    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        printf("%f\n", result[i]);
    }
}
