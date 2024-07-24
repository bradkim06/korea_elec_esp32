#include <math.h>
#include <stdio.h>
#include <string.h>

#include "filtering.h"

// 예제 입력 데이터 (여기에 실제 데이터를 사용)
float signal[SIGNAL_LENGTH];
float result[SIGNAL_LENGTH];

void app_main(void) {
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        signal[i] = sin(2 * M_PI * 1.0 * i / SAMPLE_RATE) +
                    sin(2 * M_PI * 0.1 * i / SAMPLE_RATE) +
                    sin(2 * M_PI * 60.0 * i / SAMPLE_RATE);
    }

    eeg_filter_init();
    eeg_filtering(signal, result);

    // 필터링된 데이터 출력
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        printf("%f\n", result[i]);
    }
}
