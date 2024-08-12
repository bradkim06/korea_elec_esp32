// Include header file
#include "Fulldowndetect.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_d2s 5 * 2 + 1
#define AXES 3
#define BUFFER_pr 5 * 2 + 1
#define MATH_PI 3.14159265358979
int acc_x = 0;
int acc_y = 1;
int acc_z = 2;
int pitchv = 0;
int rollv = 1;
// int AXES = 3;
// int BUFFER_pr = 5 * 2 + 1;
// int BUFFER_d2s = 5 * 2 + 1;

double cDiff_Acc_x = 0;
// ???? acc_x ??
double cDiff_Acc_y = 0;
// ???? acc_y ??
double cDiff_Acc_z = 0;

double diff2sum[BUFFER_d2s];
double pAcc[AXES];

int buffer_index_PRval = 0;

double mPRval[BUFFER_pr];
double PRval[BUFFER_pr][2];

// multiplication of Pitch and Roll values
int buffer_index_mPRval = 0;

int buffer_index_diff2sum = 0;

// --------------------???? ???? ???????
int THRESHOLD_acc_x = 15000;
int THRESHOLD_acc_y = 15000;
int THRESHOLD_acc_z = 60000;
double THRESHOLD_mprval = 0.1;
int THRESHOLD_d2s = 10000;
double THRESHOLD_wAvrPit = 0.4;
double THRESHOLD_wAvrRol = 0.4;

double Fulldowndetect::fDiffAcc(double cAcc[]) {
    double temp[AXES];
    temp[acc_x] = cAcc[acc_x] - pAcc[acc_x];
    temp[acc_y] = cAcc[acc_y] - pAcc[acc_y];
    temp[acc_z] = cAcc[acc_z] - pAcc[acc_z];
    cDiff_Acc_x = (temp[acc_x]);
    cDiff_Acc_y = (temp[acc_y]);
    cDiff_Acc_z = abs(temp[acc_z]);
    // ???? ?????? ???? ?????? ???ch`? z ?? ????
    return *temp;
}

double Fulldowndetect::fPRc(double acc[]) {
    // fPRc --> function pitch and roll calculation
    int minv = -2;
    int maxv = 2;
    double temp[2];
    double t_acc_x = acc[acc_x];
    double t_acc_y = acc[acc_y];
    double t_acc_z = acc[acc_z];
    temp[pitchv] = atan((-t_acc_x / (sqrt(pow(t_acc_y, 2) + pow(t_acc_z, 2))) *
                         (180 / MATH_PI)));
    temp[rollv] = atan((-t_acc_y / (sqrt(pow(t_acc_x, 2) + pow(t_acc_z, 2))) *
                        (180 / MATH_PI)));
    temp[pitchv] = (temp[pitchv] - minv) / (maxv - minv);
    temp[rollv] = (temp[rollv] - minv) / (maxv - minv);
    return *temp;
}

void Fulldowndetect::buffer_save(double cAcc[]) {
    *PRval[buffer_index_PRval++] = fPRc(cAcc);
    // pitch, roll ???
    pAcc[0] = cAcc[0];
    pAcc[1] = cAcc[1];
    pAcc[2] = cAcc[2];

    mPRval[buffer_index_mPRval++] = PRval[buffer_index_PRval - 1][pitchv] *
                                    PRval[buffer_index_PRval - 1][rollv];
    // pitch?? roll?? ?? ???
    if (buffer_index_PRval == BUFFER_pr) {
        buffer_index_PRval = 0;
    }
    if (buffer_index_mPRval == BUFFER_pr) {
        buffer_index_mPRval = 0;
    }
}

double Fulldowndetect::fdiff2sum(double diffacc[]) {
    double detect = sqrt(pow(diffacc[acc_x], 2) + pow(diffacc[acc_y], 2) +
                         pow(diffacc[acc_z], 2));
    return detect;
}

void Fulldowndetect::buffer_diff2sum(double diffacc[]) {
    diff2sum[buffer_index_diff2sum++] = fdiff2sum(diffacc);
    if (buffer_index_diff2sum == BUFFER_d2s) {
        buffer_index_diff2sum = 0;
    }
}

bool Fulldowndetect::fFDalg(double mprval) {
    // function Fall Down algorithm
    double wAvrD2s = 0;
    for (int i = 0; i < BUFFER_d2s; i++) {
        wAvrD2s += diff2sum[i];
    }
    wAvrD2s /= (double)BUFFER_d2s;
    // 1?? ?????
    if ((cDiff_Acc_z > THRESHOLD_acc_z) ||
        ((mprval > THRESHOLD_mprval) && (wAvrD2s > THRESHOLD_d2s))) {
        double wAvrPit = 0;
        double wAvrRol = 0;
        for (int i = 0; i < BUFFER_pr; i++) {
            wAvrPit += PRval[i][pitchv];
            wAvrRol += PRval[i][rollv];
        }
        wAvrPit /= (double)BUFFER_pr;
        wAvrRol /= (double)BUFFER_pr;
        // 2?? ?????
        if ((((wAvrPit > THRESHOLD_wAvrPit) || (wAvrRol > THRESHOLD_wAvrRol)) &&
             ((cDiff_Acc_x > THRESHOLD_acc_x) ||
              (cDiff_Acc_y > THRESHOLD_acc_y))) ||
            (cDiff_Acc_z > THRESHOLD_acc_z)) {
            return true;
        }
    }
    return false;
}

bool Fulldowndetect::fulldown_detect_result() {
    bool detected = false;
    int detecingPoint =
        ((buffer_index_mPRval + ((BUFFER_pr - 1) / 2) + 1) % (BUFFER_pr + 1)) -
        1;
    if (detecingPoint < ((BUFFER_pr - 1) / 2) - 1) {
        detecingPoint++;
    }
    detected = fFDalg(mPRval[detecingPoint]);
    return detected;
}
