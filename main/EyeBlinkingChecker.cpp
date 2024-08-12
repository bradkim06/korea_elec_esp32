// Include header file
#include "EyeBlinkingChecker.h"

#define cSize_buff_movingAvr 10
#define cTimerValueBasic 256
#define cSize_buff_baseline cTimerValueBasic * 5

// int cTimerValueBasic = 256;
// int cSize_buff_baseline = cTimerValueBasic * 5;
// int cSize_buff_movingAvr = 10;
double cBaseline_multiplier = 1.09;
int buff_baseline[cSize_buff_baseline];
int buff_movingAvr[cSize_buff_movingAvr];
double prev_movingAvrValue = 0;
double curr_movingAvrValue = 0;
int prev_sum_movingAvrValue = 0;
int curr_sum_movingAvrValue = 0;
double prev_avr_baselineValue = 0;
double curr_avr_baselineValue = 0;
int prev_sum_baselineValue = 0;
int curr_sum_baselineValue = 0;
double prev_state_value = 0;
double curr_state_value = 0;
int range_division = 0;
int idx_buff_baseline = 0;
int idx_buff_movingAvr = 0;
int gate_buff_baseline = 0;
int gate_count = 0;
int count_on = 0;
int count_count = 0;
int output[2];
int PrevEEG = 0;

int* EyeBlinkingChecker::f_EB_Checker(int ch1, int ch2) {
    int tmp_pre_buff_baseline = 0;
    int tmp_pre_buff_movingAvr = 0;
    int tCurrEEG = ch2;
    if (tCurrEEG > 700) {
        output[0] = -1;
        output[1] = 0;
        return output;
    }
    tmp_pre_buff_baseline = buff_baseline[idx_buff_baseline];
    buff_baseline[idx_buff_baseline++] = tCurrEEG;
    if (idx_buff_baseline == cSize_buff_baseline) {
        idx_buff_baseline = 0;
    }
    range_division++;
    if (range_division >= cSize_buff_baseline) {
        range_division = cSize_buff_baseline;
    }
    curr_sum_baselineValue =
        prev_sum_baselineValue + tCurrEEG - tmp_pre_buff_baseline;
    prev_sum_baselineValue = curr_sum_baselineValue;
    curr_avr_baselineValue = curr_sum_baselineValue / range_division;
    prev_avr_baselineValue = curr_avr_baselineValue;
    tmp_pre_buff_movingAvr = buff_movingAvr[idx_buff_movingAvr];
    buff_movingAvr[idx_buff_movingAvr++] = tCurrEEG;
    if (idx_buff_movingAvr == cSize_buff_movingAvr) {
        idx_buff_movingAvr = 0;
    }
    curr_sum_movingAvrValue =
        prev_sum_movingAvrValue + tCurrEEG - tmp_pre_buff_movingAvr;
    prev_sum_movingAvrValue = curr_sum_movingAvrValue;
    curr_movingAvrValue = curr_sum_movingAvrValue / cSize_buff_movingAvr;
    prev_movingAvrValue = curr_movingAvrValue;
    if (range_division >= cTimerValueBasic * 2) {
        curr_state_value = curr_movingAvrValue -
                           curr_avr_baselineValue * (cBaseline_multiplier);
        if (curr_state_value > 0) {
            if (curr_state_value < prev_state_value && gate_count == 0) {
                gate_count = 1;
                count_on++;
            }
            prev_state_value = curr_state_value;
        } else {
            gate_count = 0;
            prev_state_value = 0;
        }
        if (count_on == 1) {
            count_count++;
            output[0] = 1;
            output[1] = count_count;
        } else if (count_on == 2) {
            count_on = 1;
            output[0] = 2;
            output[1] = count_count;
            count_count = 0;
        }
        if (output[1] < 100) {
            output[0] = -2;
            output[1] = 0;
        }
        return output;
    } else {
        output[0] = -1;
        output[1] = 0;
        return output;
    }
}