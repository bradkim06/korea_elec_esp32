#include "LeadOff.h"

#include <time.h>

#include "Uart.h"

static const char *TAG = "Lead Off Class";
int capopen = 0;  // 0미착용, 1착용
bool ch1_capopen = false;
bool ch2_capopen = false;
bool co_fulldwon = true;
bool iscapCheck = true;
int iscapopen_gyro_cnt;
long curTime = time(nullptr);

void LeadOff::LeadOffData(uint8_t raw_data[], int protocol_mode) {
    if (raw_data[12] == 0x04)  // No wear
    {
        if ((raw_data[11] & 0x03) > 0)  // channel 0
        {
            if (protocol_mode == P2_12b_PROTOCOL)
                raw_data[0] = 0x0F;
            else
                raw_data[0] = 0x03;
            raw_data[1] = 0xFF;
            ch1_capopen = true;
        }
        if ((raw_data[11] & 0x0C) > 0)  // channel 1
        {
            if (protocol_mode == P2_12b_PROTOCOL)
                raw_data[2] = 0x0F;
            else
                raw_data[2] = 0x03;
            raw_data[3] = 0xFF;
            ch2_capopen = true;
        }
        if (ch2_capopen && ch1_capopen) {
            capopen = 0;
            if(!iscapCheck){
                long regTime = time(nullptr);
                long measure_time = (regTime - curTime);
                if (measure_time > checkCapOpenTime) {
                    send_msg = 1;
                    iscapCheck = true;
                }
            }
            if (co_fulldwon ) {
                long regTime = time(nullptr);
                long measure_time = (regTime - curTime);
                if (measure_time > checkFallDownTime) {
                    co_fulldwon = false;
                }
            }
        }
    } else {
        capopen = 1;  // 착용

        if (iscapCheck) {
            send_msg = 1;
            iscapCheck = false;
        }
        if (co_fulldwon) {
            curTime = time(nullptr);
        }
        if (iscapopen_gyro_cnt > 60) {
            co_fulldwon = true;
            iscapopen_gyro_cnt = 0;
        }
        iscapopen_gyro_cnt++;
        ch2_capopen = false;
        ch1_capopen = false;
    }
}

int LeadOff::getIsCapOpen() { return capopen; }

bool LeadOff::getIsFallDown() { return co_fulldwon; }

int LeadOff::getMsgSignal() { return send_msg; }

void LeadOff::setMsgSignal(int send_msg) { this->send_msg = send_msg; }

void LeadOff::setTime(int checkCapOpenTime, int checkFallDownTime) {
    this->checkCapOpenTime = checkCapOpenTime;
    this->checkFallDownTime = checkFallDownTime;
}
