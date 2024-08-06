#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_SECTIONS 2
#define PADTYPE 3 * NUM_SECTIONS  // Padding length

const float sos_coeffs[NUM_SECTIONS][6] = {
    {0.60757643f, 1.21515285f, 0.60757643f, 1.00000000f, 1.06642387f,
     0.38083453f},
    {1.00000000f, -2.00000000f, 1.00000000f, 1.00000000f, -1.98611605f,
     0.9862121f}};

void sosfilt(const float sos[6], const float *x, float *y, int n) {
    float w0 = 0, w1 = 0;
    for (int i = 0; i < n; i++) {
        float xi = x[i];
        float wi = xi - sos[4] * w0 - sos[5] * w1;
        y[i] = sos[0] * wi + sos[1] * w0 + sos[2] * w1;
        w1 = w0;
        w0 = wi;
    }
}

void sosfiltfilt(const float *x, float *y, int n) {
    int padded_length = n + 2 * PADTYPE;
    float *padded_signal = (float *)malloc(padded_length * sizeof(float));
    float *temp1 = (float *)malloc(padded_length * sizeof(float));
    float *temp2 = (float *)malloc(padded_length * sizeof(float));
    if (!padded_signal || !temp1 || !temp2) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    // Apply padding
    for (int i = 0; i < PADTYPE; i++) {
        padded_signal[i] = 2 * x[0] - x[PADTYPE - i];
        padded_signal[padded_length - 1 - i] = 2 * x[n - 1] - x[n - 2 - i];
    }
    memcpy(padded_signal + PADTYPE, x, n * sizeof(float));

    // Forward filter
    for (int section = 0; section < NUM_SECTIONS; section++) {
        sosfilt(sos_coeffs[section], (section == 0) ? padded_signal : temp1,
                temp2, padded_length);
        float *swap = temp1;
        temp1 = temp2;
        temp2 = swap;
    }

    // Reverse the array for backward filter
    for (int i = 0; i < padded_length / 2; i++) {
        float tmp = temp1[i];
        temp1[i] = temp1[padded_length - 1 - i];
        temp1[padded_length - 1 - i] = tmp;
    }

    // Backward filter
    for (int section = 0; section < NUM_SECTIONS; section++) {
        sosfilt(sos_coeffs[section], temp1, temp2, padded_length);
        float *swap = temp1;
        temp1 = temp2;
        temp2 = swap;
    }

    // Reverse the array back to original order
    for (int i = 0; i < padded_length / 2; i++) {
        float tmp = temp1[i];
        temp1[i] = temp1[padded_length - 1 - i];
        temp1[padded_length - 1 - i] = tmp;
    }

    // Copy the result to the output array, removing the padding
    memcpy(y, temp1 + PADTYPE, n * sizeof(float));

    free(padded_signal);
    free(temp1);
    free(temp2);
}

void apply_iir_filter(float *input_signal, float *output_signal,
                      int signal_length) {
    sosfiltfilt(input_signal, output_signal, signal_length);
}
