// Include header file
#include "Bpmpulse.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CPEAKARRSIZE 5

double cWeight_previous = 0.65;  // ���� BPM�� ���� ����ġ
int cMinMarginBPM = 20;  // BPM�� �ѹ��� ���� �� �ִ� �ִ�ġ
int cMaxBPM = 200;    // BPM �ִ� �Ѱ�ġ
int cMinBPM = 40;     // BPM �ּ� �Ѱ�ġ
int cWindowSize = 1;  // �̵������� ũ�� for vLowWarning
// int cPeakArrSize = 5; // ��ũ���� �����ϴ�
// ����� ũ��
int cInitMinSignalLevel = 600;  // �ʱ� �ּ� ��ȣ ���� ��
int cMaxPeakValue = 800;        // �ִ� �ּ� ��ȣ ����
int cInitSkipCountValue = 256 * 5;  // ��ŵ ī���� 5��

int vPreHV = cInitMinSignalLevel;  // ���� PPG���� ��, �ʱⰪ�� �ּҷ�����
int vCurrDiffv = 0;  // ���� HV ���а�
int vPreDiffHV = -1;  // ���� ���� ��. �ʱⰪ�� ���� ����.
int vLowWarning =
    0;  // ��ȣ�� ���� �������� �ƴϸ� �ּ� cMinSignalLevel�̻� ���������� ��Ÿ���� ����
int vCountChk = cWindowSize;  // �̵������� ũ�⸦ ī��Ʈ�� ��ü��
int vTrigg = 0;  // BPM ����� ���� �� ������ �����ϱ� ���� ����
int vCountTrigg = 0;  // BPM ������ ���̸� ī��Ʈ ��
double vCurrBPM = 0.0;  // vCountTrigg ���� ������� ���� ���� BPM
double vGabBPM = 0.0;  // ���� BPM�� ���� ��µ� BPM���� ����
double vOutBPM = 80.0;  // ���� ��µǴ� BPM. �ʱⰪ�� ��������.
int vArrPeak[CPEAKARRSIZE];  // ��ũ���� �����ϱ� ���� �迭
int vIdxArrPeak =
    0;  // ��ũ�� ����� �ε��� ����
double vMinSignalLevel = cInitMinSignalLevel;  // �ּ� ��ȣ ����
int vSkip = 0;  // �������� ũ�� �Ͼ bpm ����� ��ŵ�ؾ� �� �� 1��
                // ���� ������.
int vSkipCount = cInitSkipCountValue;  // ��ŵ �̺�Ʈ �߻��� 5�ʵ��� bmp �������
                                       // �ʱ� ���� ��

void Bpmpulse::fResetArrPeak() {
    for (int i = 0; i < CPEAKARRSIZE; i++) {
        vArrPeak[i] = cInitMinSignalLevel;
    }
    vOutBPM = 80.0;
}
double Bpmpulse::fHB(int tCurrHV) {
    if (tCurrHV == 0) {
        vSkip = 1;
        vSkipCount = cInitSkipCountValue;
        return vOutBPM;
    } else if (vSkip == 1) {
        vSkipCount--;
        if (vSkipCount == 0) {
            vSkip = 0;
            vPreHV = cInitMinSignalLevel;
            vTrigg = 0;
            vCountTrigg = 0;
        }
        return vOutBPM;
    }

    vCurrDiffv = tCurrHV - vPreHV;
    if (vCurrDiffv < 0 && vPreDiffHV >= 0) {
        vArrPeak[vIdxArrPeak++] = vPreHV;
        int tval = 0;
        for (int i = 0; i < CPEAKARRSIZE; i++) {
            tval += vArrPeak[i];
        }
        vMinSignalLevel = 1.0 * tval / CPEAKARRSIZE;
        vMinSignalLevel += vMinSignalLevel * 0.1;
        if (vMinSignalLevel > cMaxPeakValue) {
            vMinSignalLevel = cMaxPeakValue;
        }
        if (vIdxArrPeak == CPEAKARRSIZE) {
            vIdxArrPeak = 0;
        }
        if (vPreHV >= vMinSignalLevel) {
            vLowWarning = 0;
            vCountChk = cWindowSize;
        } else {
            vCountChk--;
            if (vCountChk == 0) {
                vLowWarning = 1;
                vCountChk = cWindowSize;
            }
        }
        if (vLowWarning == 0) {
            vTrigg++;
        }
    }
    if (vTrigg == 1) {
        vCountTrigg++;
    } else if (vTrigg == 2) {
        vTrigg = 1;
        vCurrBPM = (60.0 / (vCountTrigg * (1.0 / 256.0)));
        vGabBPM = abs(vCurrBPM - vOutBPM);
        if (vGabBPM < cMinMarginBPM && vCurrBPM < cMaxBPM &&
            vCurrBPM > cMinBPM && vLowWarning == 0) {
            vOutBPM =
                vOutBPM * cWeight_previous + vCurrBPM * (1 - cWeight_previous);
            vCountTrigg = 0;
        }
        if (vCurrBPM < cMinBPM) {
            vTrigg = 0;
            vCountTrigg = 0;
        }
    }
    vPreHV = tCurrHV;
    vPreDiffHV = vCurrDiffv;
    return vOutBPM;
}
