#include <math.h>
#include <stdio.h>

#include "eeg_signal.h"
#include "fir_filter.h"
#include "signal_processing.h"

float filtered_signal[BLOCK_SIZE];

void app_main() {
    init_fir_filter();

    // // 예제 데이터 시그널
    // for (int i = 0; i < BLOCK_SIZE; i++) {
    //     eeg_signal[i] = sin(2 * M_PI * 1.0 * i / SAMPLE_RATE) +
    //                     sin(2 * M_PI * 0.1 * i / SAMPLE_RATE) +
    //                     sin(2 * M_PI * 0.2 * i / SAMPLE_RATE) +
    //                     sin(2 * M_PI * 0.3 * i / SAMPLE_RATE) +
    //                     sin(2 * M_PI * 60 * i / SAMPLE_RATE) +
    //                     sin(2 * M_PI * 100.0 * i / SAMPLE_RATE);
    // }

    apply_fir_filter(eeg_signal, filtered_signal, BLOCK_SIZE);

    // Output filtered signal
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%f\n", filtered_signal[i]);
    }
}
