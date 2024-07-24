#include "filtering.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_dsp.h"
#include "esp_system.h"

EEGFilter *eeg_filter;

// FIR filter coefficients (50Hz Lowpass)
const float fir_coefficients[NUMTAPS] = {
    -0.0007276145f, -0.0020425848f, -0.0007143035f, 0.0037111003f,
    0.0054101302f,  -0.0028714177f, -0.0140885518f, -0.0075084792f,
    0.0198628140f,  0.0319939320f,  -0.0072102712f, -0.0663312162f,
    -0.0498774739f, 0.0971884874f,  0.2974154649f,  0.3915799685f,
    0.2974154649f,  0.0971884874f,  -0.0498774739f, -0.0663312162f,
    -0.0072102712f, 0.0319939320f,  0.0198628140f,  -0.0075084792f,
    -0.0140885518f, -0.0028714177f, 0.0054101302f,  0.0037111003f,
    -0.0007143035f, -0.0020425848f, -0.0007276145f};

// Butterworth Filter (0.5Hz Highpass)
const float b_coeffs[IIR_FILTER_ORDER + 1] = {0.9913600345f, -1.9827200689f,
                                              0.9913600345f};
const float a_coeffs[IIR_FILTER_ORDER + 1] = {1.0000000000f, -1.9826454185f,
                                              0.9827947193f};

// Filter state variables
static float x[IIR_FILTER_ORDER + 1] = {0};  // Input samples
static float y[IIR_FILTER_ORDER + 1] = {0};  // Output samples

static void apply_iir_filter(float *input, float *output, int block_size) {
    // Initialize state
    memset(x, 0, sizeof(x));
    memset(y, 0, sizeof(y));

    for (int n = 0; n < block_size; n++) {
        // Shift state arrays
        for (int i = IIR_FILTER_ORDER; i > 0; i--) {
            x[i] = x[i - 1];
            y[i] = y[i - 1];
        }
        x[0] = input[n];

        // Calculate filter output
        y[0] = b_coeffs[0] * x[0];
        for (int i = 1; i <= IIR_FILTER_ORDER; i++) {
            y[0] += b_coeffs[i] * x[i] - a_coeffs[i] * y[i];
        }

        output[n] = y[0];
    }
}

static void zero_phase_fir_filter(fir_f32_t *fir, float *input, float *output,
                                  int length) {
    int padded_length = length + NUMTAPS - 1;
    float *padded_input = (float *)malloc(padded_length * sizeof(float));
    float *temp = (float *)malloc(padded_length * sizeof(float));
    if (padded_input == NULL || temp == NULL) {
        printf("Memory allocation failed\n");
        free(padded_input);
        free(temp);
        return;
    }

    // Zero padding the input signal
    memset(padded_input, 0, (NUMTAPS / 2) * sizeof(float));
    memcpy(padded_input + (NUMTAPS / 2), input, length * sizeof(float));
    memset(padded_input + (NUMTAPS / 2) + length, 0,
           (NUMTAPS / 2) * sizeof(float));

    // Forward filtering
    dsps_fir_f32_ae32(fir, padded_input, temp, padded_length);

    // Reverse the result
    for (int i = 0; i < padded_length; i++) {
        padded_input[i] = temp[padded_length - 1 - i];
    }

    // Backward filtering
    dsps_fir_f32_ae32(fir, padded_input, temp, padded_length);

    // Reverse the final result
    for (int i = 0; i < padded_length; i++) {
        padded_input[i] = temp[padded_length - 1 - i];
    }

    // Remove padding and copy to output array
    memcpy(output, padded_input + (NUMTAPS / 2), length * sizeof(float));

    free(padded_input);
    free(temp);
}

void eeg_filter_init() {
    EEGFilter *filter = (EEGFilter *)malloc(sizeof(EEGFilter));
    if (filter == NULL) {
        printf("Memory allocation failed for EEGFilter\n");
        return;
    }

    filter->output_data = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
    if (filter->output_data == NULL) {
        printf("Memory allocation failed for output_data\n");
        free(filter);
        return;
    }

    filter->fir_coeffs = (float *)malloc(NUMTAPS * sizeof(float));
    if (filter->fir_coeffs == NULL) {
        printf("Memory allocation failed for fir_coeffs\n");
        free(filter->output_data);
        free(filter);
        return;
    }

    memcpy(filter->fir_coeffs, fir_coefficients, NUMTAPS * sizeof(float));
    memset(filter->fir_state, 0, NUMTAPS * sizeof(float));

    dsps_fir_init_f32(&filter->fir, filter->fir_coeffs, filter->fir_state,
                      NUMTAPS);

    eeg_filter = filter;
}

void eeg_filter_cleanup() {
    if (eeg_filter) {
        free(eeg_filter->output_data);
        free(eeg_filter->fir_coeffs);
        free(eeg_filter);
    }
}

void eeg_filtering(float *input, float *result) {
    if (eeg_filter == NULL) {
        printf("EEGFilter is not initialized\n");
        return;
    }

    // Apply IIR filter
    apply_iir_filter(input, eeg_filter->output_data, BLOCK_SIZE);

    // Apply zero-phase FIR filter
    zero_phase_fir_filter(&eeg_filter->fir, eeg_filter->output_data, result,
                          SIGNAL_LENGTH);
}
