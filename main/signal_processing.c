#include "signal_processing.h"

#include <math.h>
#include <stdio.h>

// 이상치 제거 함수
static void remove_outliers(float* input_signal, float* smoothed_signal,
                            int length, float threshold) {
    for (int i = 0; i < length; i++) {
        if (fabs(input_signal[i] - smoothed_signal[i]) > threshold) {
            input_signal[i] = smoothed_signal[i];
        }
    }
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

// 이상치 처리 함수 (3-시그마 규칙 사용)
void handle_outliers(float* data, int length) {
    float mean = 0, std_dev = 0;
    for (int i = 0; i < length; i++) {
        mean += data[i];
    }
    mean /= length;
    for (int i = 0; i < length; i++) {
        float diff = data[i] - mean;
        std_dev += diff * diff;
    }
    std_dev = sqrt(std_dev / length);
    float threshold = 3 * std_dev;
    for (int i = 0; i < length; i++) {
        if (fabs(data[i] - mean) > threshold) {
            data[i] = (data[i] > mean) ? mean + threshold : mean - threshold;
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

// NaN 값을 선형 보간으로 처리하는 함수
void interpolate_nan(float* data, int length) {
    int last_valid = -1;
    for (int i = 0; i < length; i++) {
        if (!isnan(data[i])) {
            if (last_valid != -1) {
                for (int j = last_valid + 1; j < i; j++) {
                    float t = (float)(j - last_valid) / (i - last_valid);
                    data[j] = data[last_valid] * (1 - t) + data[i] * t;
                }
            }
            last_valid = i;
        }
    }
    // 마지막 NaN 값들 처리
    for (int i = last_valid + 1; i < length; i++) {
        data[i] = data[last_valid];
    }
}

// 데이터 정규화 함수
void normalize_data(float* data, int length) {
    float mean = 0, std_dev = 0;
    for (int i = 0; i < length; i++) {
        mean += data[i];
    }
    mean /= length;
    for (int i = 0; i < length; i++) {
        float diff = data[i] - mean;
        std_dev += diff * diff;
    }
    std_dev = sqrt(std_dev / length);
    for (int i = 0; i < length; i++) {
        data[i] = (data[i] - mean) / std_dev;
    }
}

// DC 오프셋 제거 함수
void remove_dc_offset(float* data, int length) {
    float mean = 0;
    for (int i = 0; i < length; i++) {
        mean += data[i];
    }
    mean /= length;
    for (int i = 0; i < length; i++) {
        data[i] -= mean;
    }
}

// 신호를 전처리하는 함수
void preprocess_signal(float* signal, int length) {
    const int WINDOW_SIZE = 5;
    const float THRESHOLD = 0.5f;

    // NaN 값 처리
    interpolate_nan(signal, length);

    remove_dc_offset(signal, length);

    remove_baseline_drift(signal, length);

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
