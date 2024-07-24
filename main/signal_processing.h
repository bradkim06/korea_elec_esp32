#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

float calculate_mean_abs(float *data, int data_length);
void moving_average(const float *signal, float *result, int signal_length,
                    int window_size);
void remove_baseline_drift(const float *signal, float *result,
                           int signal_length, int window_size,
                           float drift_ratio);

#endif  // SIGNAL_PROCESSING_H
