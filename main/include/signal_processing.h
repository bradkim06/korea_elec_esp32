#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

void moving_average(float* input_signal, float* output_signal, int length,
                    int window_size);
void remove_baseline_drift(float* signal, int length);
void preprocess_signal(float* signal, int length);
void interpolate_nan(float* data, int length);
void normalize_data(float* data, int length);
void remove_dc_offset(float* data, int length);
void handle_outliers(float* data, int length);

#endif  // SIGNAL_PROCESSING_H
