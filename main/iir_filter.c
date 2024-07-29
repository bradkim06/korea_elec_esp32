#include "iir_filter.h"

#include <stdio.h>
#include <string.h>

#include "signal_processing.h"

#define NUM_SECTIONS 4

const float sos_coeffs[NUM_SECTIONS][6] = {
    {0.0418642627f, 0.0837285253f, 0.0418642627f, 1.0000000000f, -0.3758657033f,
     0.0734859039f},
    {1.0000000000f, 2.0000000000f, 1.0000000000f, 1.0000000000f, -0.5006273934f,
     0.4763892002f},
    {1.0000000000f, -2.0000000000f, 1.0000000000f, 1.0000000000f,
     -1.9771505400f, 0.9773039671f},
    {1.0000000000f, -2.0000000000f, 1.0000000000f, 1.0000000000f,
     -1.9906166421f, 0.9907672893f},
};

// 필터 상태 구조체
typedef struct {
    float z[NUM_SECTIONS][2];
} IIRFilterState;

static IIRFilterState forward_state, backward_state;

static void iir_filter_apply(IIRFilterState* state, const float* input,
                             float* output, int length) {
    for (int n = 0; n < length; n++) {
        float x = input[n];
        for (int i = 0; i < NUM_SECTIONS; i++) {
            float* z = state->z[i];
            const float* sos = sos_coeffs[i];
            float y = sos[0] * x + z[0];
            z[0] = sos[1] * x + z[1] - sos[4] * y;
            z[1] = sos[2] * x - sos[5] * y;
            x = y;
        }
        output[n] = x;
    }
}

static void iir_filter_apply_reverse(IIRFilterState* state, const float* input,
                                     float* output, int length) {
    for (int n = length - 1; n >= 0; n--) {
        float x = input[n];
        for (int i = 0; i < NUM_SECTIONS; i++) {
            float* z = state->z[i];
            const float* sos = sos_coeffs[i];
            float y = sos[0] * x + z[0];
            z[0] = sos[1] * x + z[1] - sos[4] * y;
            z[1] = sos[2] * x - sos[5] * y;
            x = y;
        }
        output[n] = x;
    }
}

void sosfiltfilt(const float* input, float* output, int length) {
    float* temp = (float*)malloc(length * sizeof(float));
    if (temp == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // Forward filtering
    iir_filter_apply(&forward_state, input, temp, length);

    // Reverse filtering
    iir_filter_apply_reverse(&backward_state, temp, output, length);

    free(temp);
}

void apply_iir_filter(float* input_signal, float* output_signal,
                      int signal_length) {
    // NaN 값 처리
    interpolate_nan(input_signal, signal_length);

    remove_dc_offset(input_signal, signal_length);

    // 이상치 처리
    handle_outliers(input_signal, signal_length);

    // 신호 전처리
    preprocess_signal(input_signal, signal_length);

    // 데이터 정규화
    normalize_data(input_signal, signal_length);

    // 필터 적용
    sosfiltfilt(input_signal, output_signal, signal_length);
}
