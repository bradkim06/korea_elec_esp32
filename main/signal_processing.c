#include "signal_processing.h"

#include <math.h>
#include <stdio.h>

float calculate_mean_abs(float *data, int data_length) {
    float sum = 0.0f;
    for (int i = 0; i < data_length; i++) {
        sum += fabsf(data[i]);
    }
    return sum / data_length;
}

void moving_average(const float *signal, float *result, int signal_length,
                    int window_size) {
    float sum = 0;
    int i;
    int half_window = window_size / 2;

    // 초기 윈도우 합계 계산
    for (i = 0; i < window_size && i < signal_length; i++) {
        sum += signal[i];
    }

    // 첫 번째 결과 계산
    result[0] = sum / window_size;

    // 나머지 결과 계산
    for (i = 1; i < signal_length; i++) {
        // 윈도우에서 벗어난 값 제거
        if (i - half_window - 1 >= 0) {
            sum -= signal[i - half_window - 1];
        }
        // 새로운 값 추가
        if (i + half_window < signal_length) {
            sum += signal[i + half_window];
        }
        result[i] = sum / window_size;
    }
}

void remove_baseline_drift(const float *signal, float *result,
                           int signal_length, int window_size,
                           float drift_ratio) {
    printf("test\n");
    // 이동 평균 계산
    moving_average(signal, result, signal_length, window_size);

    // baseline drift 제거
    for (int i = 0; i < signal_length; i++) {
        result[i] = signal[i] - drift_ratio * result[i];
    }
}
