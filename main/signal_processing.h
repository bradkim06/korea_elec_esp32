#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

float calculate_mean_abs(float* data, int data_length);
void moving_average(float* input_signal, float* output_signal, int length,
                    int window_size);
void remove_outliers(float* input_signal, float* smoothed_signal, int length,
                     float threshold);
void remove_baseline_drift(float* signal, int length);
void preprocess_signal(float* signal, int length);

#endif  // SIGNAL_PROCESSING_H
