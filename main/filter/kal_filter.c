// Kalman Filter 구조체 정의
typedef struct {
    float transition_matrix;       // A
    float observation_matrix;      // C
    float state_estimate;          // x_hat
    float estimate_covariance;     // P
    float observation_covariance;  // R
    float transition_covariance;   // Q
} KalmanFilter;

// Kalman Filter 초기화 함수
static void kalman_filter_init(KalmanFilter *kf, float transition_matrix,
                               float observation_matrix,
                               float initial_state_estimate,
                               float initial_estimate_covariance,
                               float observation_covariance,
                               float transition_covariance) {
    kf->transition_matrix = transition_matrix;
    kf->observation_matrix = observation_matrix;
    kf->state_estimate = initial_state_estimate;
    kf->estimate_covariance = initial_estimate_covariance;
    kf->observation_covariance = observation_covariance;
    kf->transition_covariance = transition_covariance;
}

// Kalman Filter 업데이트 함수
static void kalman_filter_update(KalmanFilter *kf, float measurement) {
    // Prediction update
    float predicted_state_estimate = kf->transition_matrix * kf->state_estimate;
    float predicted_estimate_covariance = kf->transition_matrix *
                                              kf->estimate_covariance *
                                              kf->transition_matrix +
                                          kf->transition_covariance;

    // Measurement update
    float innovation =
        measurement - kf->observation_matrix * predicted_state_estimate;
    float innovation_covariance = kf->observation_matrix *
                                      predicted_estimate_covariance *
                                      kf->observation_matrix +
                                  kf->observation_covariance;

    // Kalman gain
    float kalman_gain = predicted_estimate_covariance * kf->observation_matrix /
                        innovation_covariance;

    // Update state estimate and estimate covariance
    kf->state_estimate = predicted_state_estimate + kalman_gain * innovation;
    kf->estimate_covariance = (1 - kalman_gain * kf->observation_matrix) *
                              predicted_estimate_covariance;
}

// Kalman Filter 적용 함수
void apply_kalman_filter(float *data, int data_length) {
    KalmanFilter kf;
    kalman_filter_init(&kf, 1.0, 1.0, 0.0, 1.0, 0.4, 0.01);

    for (int i = 0; i < data_length; i++) {
        kalman_filter_update(&kf, data[i]);
        data[i] = kf.state_estimate;
    }
}
