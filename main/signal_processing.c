#include "signal_processing.h"

#include <math.h>
#include <stdio.h>

float calculate_mean_abs(float* data, int data_length) {
    float sum = 0.0f;
    for (int i = 0; i < data_length; i++) {
        sum += fabsf(data[i]);
    }
    return sum / data_length;
}

// 이동평균 계산 함수
void moving_average(float* input_signal, float* output_signal, int length,
                    int window_size) {
    float window_sum = 0.0;
    int half_window = window_size / 2;

    // 초기 윈도우 합 계산
    for (int i = 0; i < window_size; i++) {
        window_sum += input_signal[i];
    }

    for (int i = 0; i < length; i++) {
        if (i >= half_window && i < length - half_window) {
            output_signal[i] = window_sum / window_size;
            window_sum += input_signal[i + half_window + 1] -
                          input_signal[i - half_window];
        } else {
            output_signal[i] = input_signal[i];
        }
    }
}

// 이상치 제거 함수
void remove_outliers(float* input_signal, float* smoothed_signal, int length,
                     float threshold) {
    for (int i = 0; i < length; i++) {
        if (fabs(input_signal[i] - smoothed_signal[i]) > threshold) {
            input_signal[i] = smoothed_signal[i];
        }
    }
}

// baseline drift 제거 함수
void remove_baseline_drift(float* signal, int length) {
    const int window_size = 16;
    const float drift_ratio = 0.5;

    float* rolling_mean = (float*)malloc(length * sizeof(float));
    if (rolling_mean == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // 이동평균 필터 적용
    moving_average(signal, rolling_mean, length, window_size);

    // baseline drift의 일부만 제거
    for (int i = 0; i < length; i++) {
        signal[i] = signal[i] - drift_ratio * rolling_mean[i];
    }

    free(rolling_mean);
}

// 신호를 전처리하는 함수
void preprocess_signal(float* signal, int length) {
    const int WINDOW_SIZE = 5;
    const float THRESHOLD = 0.5f;

    float* smoothed_signal = (float*)malloc(length * sizeof(float));
    if (smoothed_signal == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // 이동평균 적용
    moving_average(signal, smoothed_signal, length, WINDOW_SIZE);

    // 이상치 제거
    remove_outliers(signal, smoothed_signal, length, THRESHOLD);

    free(smoothed_signal);
}
