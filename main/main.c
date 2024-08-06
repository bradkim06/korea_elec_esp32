#include <math.h>

#include "bandpower.h"
#include "eeg_signal.h"
#include "sfft.h"

__attribute__((aligned(16))) float input_signal[N_SAMPLES];

static void generate_sine_wave(float *signal, int length, float sample_rate) {
    for (int i = 0; i < length; i++) {
        signal[i] = sinf(2 * M_PI * 5.0f * i / sample_rate) +
                    sinf(2 * M_PI * 100.0f * i / sample_rate) +
                    sinf(2 * M_PI * 10.0f * i / sample_rate) +
                    sinf(2 * M_PI * 15.0f * i / sample_rate) +
                    sinf(2 * M_PI * 0.1f * i / sample_rate);
        // signal[i] = eeg_signal[i];
    }
}

void app_main() {
    generate_sine_wave(input_signal, N_SAMPLES, N_FFT);

    init_bandpower();
    bandpower(input_signal);
}
