#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_SECTIONS 2

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

void pad_signal(const float *x, float *padded_signal, int n, int padlen,
                const char *padtype) {
    if (strcmp(padtype, "odd") == 0) {
        for (int i = 0; i < padlen; i++) {
            padded_signal[i] = 2 * x[0] - x[padlen - i];
            padded_signal[n + padlen + i] = 2 * x[n - 1] - x[n - 2 - i];
        }
    } else if (strcmp(padtype, "even") == 0) {
        for (int i = 0; i < padlen; i++) {
            padded_signal[i] = x[padlen - i];
            padded_signal[n + padlen + i] = x[n - 2 - i];
        }
    } else if (strcmp(padtype, "constant") == 0) {
        for (int i = 0; i < padlen; i++) {
            padded_signal[i] = x[0];
            padded_signal[n + padlen + i] = x[n - 1];
        }
    }
    memcpy(padded_signal + padlen, x, n * sizeof(float));
}

void sosfiltfilt(float *x, int n, const char *padtype, int padlen) {
    int padded_length = n + 2 * padlen;
    float *padded_signal = (float *)malloc(padded_length * sizeof(float));
    float *temp1 = (float *)malloc(padded_length * sizeof(float));
    float *temp2 = (float *)malloc(padded_length * sizeof(float));
    if (!padded_signal || !temp1 || !temp2) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    // Apply padding
    pad_signal(x, padded_signal, n, padlen, padtype);

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
    memcpy(x, temp1 + padlen, n * sizeof(float));

    free(padded_signal);
    free(temp1);
    free(temp2);
}

void apply_iir_filter(float *input_signal, int signal_length) {
    const int padlen = 3 * NUM_SECTIONS;  // Default padlen
    const char *padtype = "odd";          // Default padtype
    sosfiltfilt(input_signal, signal_length, padtype, padlen);
}
