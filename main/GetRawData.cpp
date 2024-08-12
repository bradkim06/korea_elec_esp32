#include "GetRawData.h"

#include "Uart.h"

static const char *TAG = "GetRawData Class";

int in_data_pos = 0;
int m_samplerate = 256;
// eeg ppg data value
int eeg_ch1, eeg_ch2, ppg_ch3;

// temperature data value
int b_Temp_high, b_Temp_low, mlx_pec;
int tempData[2];

// gyro data value
double acc[3] = {0}, gyro[3] = {0};

// batterydata
int battery_data = 0;

int sendCount = 0;
void GetRawData::DataSort(uint8_t raw_data[], int protocol_mode) {
    int jj;
    int ch;
    int max_channel;
    max_channel = 6;  // P2 Max channel
    if (max_channel > EEG_MAX_CHANNEL) max_channel = EEG_MAX_CHANNEL;

    switch (raw_data[13] % 4) {
        case 0: {
            battery_data = (unsigned int)((raw_data[6] & 0xFF) << 8) |
                           (raw_data[7] & 0xFF);
            break;
        }
        case 1: {
            acc[0] = (short)((raw_data[6] & 0xFF) << 8) | (raw_data[7] & 0xFF);
            acc[1] = (short)((raw_data[8] & 0xFF) << 8) | (raw_data[9] & 0xFF);
            acc[2] =
                (short)((raw_data[10] & 0xFF) << 8) | (raw_data[11] & 0xFF);
            break;
        }
        case 2: {
            gyro[0] = (short)((raw_data[6] & 0xFF) << 8) | (raw_data[7] & 0xFF);
            gyro[1] = (short)((raw_data[8] & 0xFF) << 8) | (raw_data[9] & 0xFF);
            gyro[2] =
                (short)((raw_data[10] & 0xFF) << 8) | (raw_data[11] & 0xFF);
            break;
        }
        case 3: {
            b_Temp_high = (int)(raw_data[6] & 0xFF);
            b_Temp_low = (int)(raw_data[7] & 0xFF);
            tempData[0] = (int)(((b_Temp_high & 0x007F) << 8) + b_Temp_low);
            tempData[1] = (int)(raw_data[8] & 0xFF);
            break;
        }
    }

    if (in_data_pos >= m_samplerate) {
        if (sendCount >= SEND_PERIOD) {
            msgSig = 1;
            sendCount = 0;
        }
        sendCount++;
        in_data_pos = 0;
    }
    for (ch = 0; ch < max_channel; ch++) {
        int cur_data;
        jj = 2 * ch;

        cur_data =
            (((int)(raw_data[jj] & 0xFF) << 8) | (raw_data[jj + 1] & 0xFF));

        if (protocol_mode == P2_12b_PROTOCOL) {
            // 12bits -> 10bits
            cur_data >>= 1;
            // 0 ~ 4095 ==> 0 ~ 2047
            if (cur_data > 1535)  // 1535 : 1024+511
                cur_data = 1535;
            else if (cur_data < 512)
                cur_data = 512;
            cur_data -= 512;
            // 0 ~ 1023
        }

        if (ch == 0) {
            eeg_ch1 = cur_data;
        } else if (ch == 1) {
            eeg_ch2 = cur_data;
        } else if (ch == 2) {
            ppg_ch3 = cur_data;
        } else {
        }
    }
    // ESP_LOGI(TAG, "LoraSend indatapos: %d", in_data_pos);
    in_data_pos++;
}

int GetRawData::getEegCh1Data() { return eeg_ch1; }
int GetRawData::getEegCh2Data() { return eeg_ch2; }
int GetRawData::getPpgData() { return ppg_ch3; }
int GetRawData::batteryData() { return battery_data; }
double* GetRawData::getAccData() { return acc; }
double* GetRawData::getGyroData() { return gyro; }
int* GetRawData::getTempData() { return tempData; }
int GetRawData::getPriodMsgSignal() { return msgSig; }
void GetRawData::setPriodMsgSignal(int msg) { this->msgSig = msg; }
void GetRawData::setMsgPriod(int sec) { this->SEND_PERIOD = sec; }
