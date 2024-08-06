#include "iir_filter.h"

#include <stdio.h>
#include <string.h>

#include "signal_processing.h"

#define NUM_SECTIONS 2

const float sos_coeffs[NUM_SECTIONS][6] = {
    {0.60757643f, 1.21515285f, 0.60757643f, 1.00000000f, 1.06642387f,
     0.38083453f},
    {1.00000000f, -2.00000000f, 1.00000000f, 1.00000000f, -1.98611605f,
     0.9862121f}};

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
    // 필터 적용
    sosfiltfilt(input_signal, output_signal, signal_length);
}
