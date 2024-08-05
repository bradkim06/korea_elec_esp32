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

// 합성곱 함수 정의 (mode="same")
void convolve_same(const float* signal, int signal_len, const float* v,
                   int v_len, float* result) {
    int half_v_len = v_len / 2;

    for (int i = 0; i < signal_len; i++) {
        result[i] = 0.0;
        for (int j = 0; j < v_len; j++) {
            int a_index = i + j - half_v_len;
            if (a_index >= 0 && a_index < signal_len) {
                result[i] += signal[a_index] * v[j];
            }
        }
    }
}

// // 이상치 처리 함수 (3-시그마 규칙 사용)
// static void handle_outliers(float* data, int length) {
//     float mean = 0, std_dev = 0;
//     for (int i = 0; i < length; i++) {
//         mean += data[i];
//     }
//     mean /= length;
//     for (int i = 0; i < length; i++) {
//         float diff = data[i] - mean;
//         std_dev += diff * diff;
//     }
//     std_dev = sqrt(std_dev / length);
//     float threshold = 3 * std_dev;
//     for (int i = 0; i < length; i++) {
//         if (fabs(data[i] - mean) > threshold) {
//             data[i] = (data[i] > mean) ? mean + threshold : mean - threshold;
//         }
//     }
// }

static float* create_window(int window_size) {
    float* window = (float*)malloc(window_size * sizeof(float));
    if (window == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < window_size; i++) {
        window[i] = 1.0f / window_size;
    }

    return window;
}

// baseline drift 제거 함수
static void remove_baseline_drift(float* signal, int length) {
    const int window_size = 128;
    const float drift_ratio = 0.3;
    float* window = create_window(window_size);

    float* rolling_mean = (float*)malloc(length * sizeof(float));
    if (rolling_mean == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // 이동평균 필터 적용
    convolve_same(signal, length, window, window_size, rolling_mean);

    // baseline drift의 일부만 제거
    for (int i = 0; i < length; i++) {
        signal[i] = signal[i] - drift_ratio * rolling_mean[i];
    }

    free(rolling_mean);
    free(window);
}

// NaN 값을 선형 보간으로 처리하는 함수
static void interpolate_nan(float* data, int length) {
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

// // 데이터 정규화 함수
// static void normalize_data(float* data, int length) {
//     float mean = 0, std_dev = 0;
//     for (int i = 0; i < length; i++) {
//         mean += data[i];
//     }
//     mean /= length;
//     for (int i = 0; i < length; i++) {
//         float diff = data[i] - mean;
//         std_dev += diff * diff;
//     }
//     std_dev = sqrt(std_dev / length);
//     for (int i = 0; i < length; i++) {
//         data[i] = (data[i] - mean) / std_dev;
//     }
// }

// DC 오프셋 제거 함수
static void remove_dc_offset(float* data, int length) {
    float mean = 0;
    for (int i = 0; i < length; i++) {
        mean += fabs(data[i]);
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

    float* window = create_window(WINDOW_SIZE);
    float* smoothed_signal = (float*)malloc(length * sizeof(float));
    if (smoothed_signal == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // 이동평균 적용
    convolve_same(signal, length, window, WINDOW_SIZE, smoothed_signal);

    // 이상치 제거
    remove_outliers(signal, smoothed_signal, length, THRESHOLD);

    free(window);
    free(smoothed_signal);
}
