#ifndef BANDPOWER_H
#define BANDPOWER_H

// Define constants
#define N_SAMPLES 2560
#define N_FFT 256
#define WIN_LENGTH N_FFT
#define HOP_LENGTH 128
#define N_FRAMES (1 + (N_SAMPLES - WIN_LENGTH) / HOP_LENGTH)

int init_bandpower();
int measureFatigue(float *input_signal);

#endif
