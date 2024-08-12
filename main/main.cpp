/*
UART Interrupt Example
*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "bandpower.h"
#include "eeg_signal.h"
#include "Bpmpulse.h"
#include "EyeBlinkingChecker.h"
#include "Fulldowndetect.h"
#include "GetRawData.h"
#include "LeadOff.h"
#include "Uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

extern "C" {
void app_main();
}

static const char *TAG = "uart_events";

/**
 * This example shows how to use the UART driver to handle UART interrupt.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define leadoff_mode 1

#define __BPM_LOGS__ 1	// 0 : 넘어짐, 1: BPM, 2 : Eye blinking 로그 확인
#define UNBLINK_SEC  60 // 눈감은 유지시간

#define MSG_TOTAL_LENGTH 12 // msg length(10) + "\r\n"(2)
#define MSG_LENGTH	 10 // msg length(10)
#define MSG_START_PART	 14 // "AT+SEND=0,%d," --> %d 2byte

// bpmpulse value, class
Bpmpulse mbpmpulse = Bpmpulse();
int bpm_value = 80;
bool bpm_reset = false;

// eyeblinking
EyeBlinkingChecker meyeblinkingchecker = EyeBlinkingChecker();
bool eyeblink_detect;

// Uart Class
Uart mUart = Uart();
// LeadOff Class
LeadOff mLeadOff = LeadOff();
// GetRawData Class
GetRawData mGetRawData = GetRawData();

const int BUFFER_SIZE = 1000;
char msg[BUFFER_SIZE] = {0};
char msg_start_part[MSG_START_PART] = {0};

uint8_t raw_data[RAW_DATA_POS_MAX][31]; // 6*2ch, switch, packet_no
uint8_t raw_data_size[RAW_DATA_POS_MAX];
int raw_data_out_pos = 0;
int protocol_mode = P2_12b_PROTOCOL;

// battery
int batteryValue;
int bpm = 0;

int is_capopen = 0; // 0미착용, 1착용
bool co_fulldwon_bool = true;

// Falldown detect
Fulldowndetect mFalldowndetect = Fulldowndetect();
bool fall_down_detect;

// acc gyro Data
double *accData;
double *gyroData;
// temp array data
int *arrTemp;
float calc_temp = 0;

char hexStr[(BUFFER_SIZE * 2) + 1];
// core Task
TaskHandle_t Task1;

bool fulldown_detect_func(double acc[])
{
	double diffAcc[3];
	bool detect;
	*diffAcc = mFalldowndetect.fDiffAcc(acc);
	mFalldowndetect.buffer_save(acc);
	mFalldowndetect.buffer_diff2sum(diffAcc);
	detect = mFalldowndetect.fulldown_detect_result();
	return detect;
}

// bpm detect init
void bpm_detect_init()
{
	mbpmpulse.fResetArrPeak();
}

// bpm detect function
int bpm_detect_func(int bpm_data)
{
	return (int)floor(mbpmpulse.fHB(bpm_data));
}

// eyeblinking function
bool eye_blinking_detect_func(int ch1, int ch2)
{
	int *eyeblink_output;
	bool unblink = false;
	eyeblink_output = meyeblinkingchecker.f_EB_Checker(ch1, ch2);
	if (eyeblink_output[1] / 256 >= UNBLINK_SEC) {
		unblink = true;
	}
	return unblink;
}

// 배터리 데이터
int battery_change(int data)
{
	int output = 0;
	int temp_data = 0;
	if (data == 4095) {
		output = 100;
	} else {
		temp_data = (4095 - data) / 10;
		output = 99 - temp_data;
	}

	return output;
}

void string2hexString(char *input, char *output)
{
	int loop;
	int i;

	i = 0;
	loop = 0;

	while (input[loop] != '\0') {
		sprintf((char *)(output + i), "%02X", input[loop]);
		loop += 1;
		i += 2;
	}
	// insert NULL at the end of the output string
	output[i++] = '\0';
}

void debugmsg(char *msg)
{
	mUart.Uart_Send(EX_UART_PC, msg, strlen(msg));
}

void sendEventMsg(int uart_num)
{
	char eventSendData[(BUFFER_SIZE * 2) + 16] = {0};
	char inputData[(BUFFER_SIZE * 2) + 16] = {0};
	// sprintf(inputData,
	//         "\"b\":\"%d\",\"ev\":\"%d,%d,%d\",\"t\":\"%.2f\",\"a\":\"%d\"\r\n",
	//         bpm_value, is_capopen, fall_down_detect, eyeblink_detect, calc_temp,
	//         batteryValue);
	sprintf(inputData, "%d,%d,%d,%d,%.2f,%d", bpm_value, is_capopen, fall_down_detect,
		eyeblink_detect, calc_temp, batteryValue);
	string2hexString(inputData, hexStr);
	sprintf(eventSendData, "%s\r\n", hexStr);
	ESP_LOGI(TAG, "Recv str: %s", (char *)eventSendData);
	mUart.Uart_Send(uart_num, msg_start_part, strlen(msg_start_part));
	mUart.Uart_Send(uart_num, eventSendData, strlen(eventSendData));
	debugmsg(msg_start_part);
	debugmsg(eventSendData);
}

static void P2_DoRcvAction()
{
	int msgSignal = 0, batteryData = 0, msgPriodSignal = 0;
	int eegCh1 = 0, eegCh2 = 0, ppgCh3 = 0;

	if (leadoff_mode) {
		mLeadOff.LeadOffData(raw_data[raw_data_out_pos], protocol_mode);
		is_capopen = mLeadOff.getIsCapOpen();
		co_fulldwon_bool = mLeadOff.getIsFallDown();
		msgSignal = mLeadOff.getMsgSignal();
		if (msgSignal == 1) {
			sendEventMsg(EX_UART_LORA);
			mLeadOff.setMsgSignal(0);
		}
	}
	mGetRawData.DataSort(raw_data[raw_data_out_pos], protocol_mode);

	// battery
	batteryData = mGetRawData.batteryData();
	batteryValue = battery_change(batteryData);

	// falldown
	if (co_fulldwon_bool) {
		accData = mGetRawData.getAccData();
		fall_down_detect = fulldown_detect_func(accData);
		if (fall_down_detect) {
			sendEventMsg(EX_UART_LORA);
		}
	}

	// eeg eyblinking
	eegCh1 = mGetRawData.getEegCh1Data();
	eegCh2 = mGetRawData.getEegCh2Data();
	eyeblink_detect = eye_blinking_detect_func(eegCh1, eegCh2);
	if (eyeblink_detect) {
		sendEventMsg(EX_UART_LORA);
	}

	// bpm
	if (is_capopen == 1) {
		ppgCh3 = mGetRawData.getPpgData();
		bpm_value = bpm_detect_func(ppgCh3);
		bpm_reset = false;
		if (bpm_value > 140) {
			sendEventMsg(EX_UART_LORA);
		}
	} else if (is_capopen == 0 && bpm_reset == false) {
		bpm_detect_init();
		bpm_reset = true;
	}

	// tempdata
	arrTemp = mGetRawData.getTempData();
	double calculData = 0;
	if (arrTemp[1] < 240) {
		calculData = arrTemp[0] * 0.02;
		calculData -= 272.15;
		calculData += 0.5;
		if (calculData > 0) {
			calc_temp = calculData;
		}
	}
	msgPriodSignal = mGetRawData.getPriodMsgSignal();
	if (msgPriodSignal == 1) {
		sendEventMsg(EX_UART_LORA);
		mGetRawData.setPriodMsgSignal(0);
	}
	// 256개 쌓아서 처리해야
}

static void p2_init(void *pvParameters)
{
	int checkData = 0;
	while (true) {
		checkData = mUart.DataExistCheck(raw_data_size[raw_data_out_pos]);
		if (checkData == 1) {
			P2_DoRcvAction();
			raw_data_size[raw_data_out_pos] = 0;
			raw_data_out_pos++;
			if (raw_data_out_pos >= RAW_DATA_POS_MAX) {
				raw_data_out_pos = 0;
			}
		}
		vTaskDelay(1);
	}
}

__attribute__((aligned(16))) float input_signal[N_SAMPLES];

static void generate_sine_wave(float *signal, int length, float sample_rate)
{
	for (int i = 0; i < length; i++) {
		// signal[i] = sinf(2 * M_PI * 5.0f * i / sample_rate) +
		//             sinf(2 * M_PI * 100.0f * i / sample_rate) +
		//             sinf(2 * M_PI * 10.0f * i / sample_rate) +
		//             sinf(2 * M_PI * 15.0f * i / sample_rate) +
		//             sinf(2 * M_PI * 0.1f * i / sample_rate);
		signal[i] = eeg_signal[i];
	}
}

// static const char *TAG = "UART TEST";
void app_main(void)
{
	mUart.PcUartinit();
	mUart.Uart1init();
	mUart.Uart2init();
	bpm_detect_init();
	uint8_t *data1 = (uint8_t *)malloc(BUF_SIZE);

	generate_sine_wave(input_signal, N_SAMPLES, N_FFT);

	init_bandpower();
	measureFatigue(input_signal);

	for (int ii = 0; ii < RAW_DATA_POS_MAX; ii++) {
		raw_data_size[ii] = 0;
	}

	// printf("hello esp32!!\n");

	char t_msg_start_part[MSG_START_PART + 2] = {0}; // +2 --> null character
	//
	sprintf(t_msg_start_part, "AT+LPSEND=3:1:");
	memcpy(msg_start_part, t_msg_start_part, 14);

	xTaskCreatePinnedToCore(p2_init, "Task1", 10000, NULL, 1, &Task1, 0);

	while (1) {
		int lenLora = uart_read_bytes(EX_UART_LORA, data1, (BUF_SIZE - 1),
					      20 / portTICK_PERIOD_MS);
		if (lenLora) {
			data1[lenLora] = '\0';
			ESP_LOGI(TAG, "Recv str: %s", (char *)data1);
			mUart.Uart_Send(EX_UART_PC, (char *)data1, lenLora);
		}
	}
}
