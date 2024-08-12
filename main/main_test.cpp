/*
UART Interrupt Example
*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

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

// static const char *TAG = "uart_events";

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

#define __BPM_LOGS__ 1  // 0 : 넘어짐, 1: BPM, 2 : Eye blinking 로그 확인
#define UNBLINK_SEC 60  // 눈감은 유지시간

#define MSG_TOTAL_LENGTH 12  // msg length(10) + "\r\n"(2)
#define MSG_LENGTH 10        // msg length(10)
#define MSG_START_PART 14    // "AT+SEND=0,%d," --> %d 2byte

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

const int BUFFER_SIZE = 100;
char msg[BUFFER_SIZE] = {0};
char msg_start_part[MSG_START_PART] = {0};

uint8_t raw_data[RAW_DATA_POS_MAX][31];  // 6*2ch, switch, packet_no
uint8_t raw_data_size[RAW_DATA_POS_MAX];
int raw_data_out_pos = 0;
int protocol_mode = P2_12b_PROTOCOL;

// battery
int batteryValue;
int bpm = 0;

int is_capopen = 0;  // 0미착용, 1착용
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
// core Task
TaskHandle_t Task1;

void string2hexString(char* input, char* output)
{
    int loop;
    int i;

    i = 0;
    loop = 0;

    while (input[loop] != '\0') {
        sprintf((char*)(output + i), "%02X", input[loop]);
        loop += 1;
        i += 2;
    }
    //insert NULL at the end of the output string
    output[i++] = '\0';
}

void sendEventLoraMsg(int uart_num) {
    char eventSendData[BUFFER_SIZE] = {0};
    sprintf(eventSendData,
            "%d,%d,%d,%d,%.2f,%d\n",
            bpm_value, is_capopen, fall_down_detect, eyeblink_detect, calc_temp,
            batteryValue);
    mUart.Uart_Send(uart_num, msg_start_part, strlen(msg_start_part));
    mUart.Uart_Send(uart_num, eventSendData, strlen(eventSendData));
}

void debugmsg(char *msg) { mUart.Uart_Send(EX_UART_PC, msg, strlen(msg)); }

static const char *TAG = "UART TEST";

void app_main(void) {
    mUart.PcUartinit();
    mUart.Uart1init();
    mUart.Uart2init();
    // bpm_detect_init();
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    uint8_t *data1 = (uint8_t *) malloc(BUF_SIZE);
    int length = 0;
    // for (int ii = 0; ii < RAW_DATA_POS_MAX; ii++) {
    //     raw_data_size[ii] = 0;
    // }
    char hexStr[(BUFFER_SIZE*2)+1];

 
    // // mUart.Uart_Send(EX_UART_LORA, "AT+JOIN=1:1", 11);
    string2hexString("80,0,0,0,36.31,100",hexStr);
    // debugmsg(hexStr);
    char eventSendData[(BUFFER_SIZE*2)+16] = {0};
    sprintf(eventSendData,"%s\r\n",hexStr);
    char t_msg_start_part[MSG_START_PART + 2] = {0};  // +2 --> null character
    // //
    sprintf(t_msg_start_part, "AT+LPSEND=3:1:");
    memcpy(msg_start_part, t_msg_start_part, 14);
    while(1){
        // mUart.Uart_Send(EX_UART_LORA, msg_start_part, strlen(msg_start_part));
        int len = uart_read_bytes(EX_UART_PC, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (len) {
            data[len] = '\0';
            ESP_LOGI(TAG, "LoraSend str: %s", (char *) data);
            if( (char) data[0] == 'a'){
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT\r\n", 8);
            }else if((char) data[0] == 'n')
            {
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+NJM=?\r\n", 18);

            }else if((char) data[0] == 'd')
            {
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+DEVEUI=?\r\n", 18);

            }else if((char) data[0] == 'q')
            {
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+APPEUI=?\r\n", 18);

            }else if((char) data[0] == 'k')
            {
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+APPKEY=?\r\n", 18);

            }
            else if((char) data[0] == 'j')
            {
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+JOIN=1:1\r\n", 18);

            }else if((char) data[0] == 'p'){
                ESP_LOGI(TAG, "LoraSend: %s", (char *) data);
                mUart.Uart_Send(EX_UART_LORA, "AT+PNM=?\r\n", 18);
            }
            else if((char) data[0] == 'b')
            {
                ESP_LOGI(TAG, "LoraSend str: %s", (char *) eventSendData);
                ESP_LOGI(TAG, "b str: %c", (char) data[0]);
                mUart.Uart_Send(EX_UART_PC, msg_start_part, strlen(msg_start_part));
                mUart.Uart_Send(EX_UART_PC, eventSendData, strlen(eventSendData));
                mUart.Uart_Send(EX_UART_LORA, msg_start_part, strlen(msg_start_part));
                mUart.Uart_Send(EX_UART_LORA, eventSendData, strlen(eventSendData));

            }else{
                mUart.Uart_Send(EX_UART_LORA, "AT+SEND=12:123456\r\n", 18);
            }

        }   
       
        int lenLora= uart_read_bytes(EX_UART_LORA, data1, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        if (lenLora) {
            data1[lenLora] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (char *) data1);
            mUart.Uart_Send(EX_UART_PC, (char *) data1, lenLora);
        }   

        // debugmsg(hexStr);
        // vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    // xTaskCreatePinnedToCore(p2_init, "Task1", 10000, NULL, 1, &Task1, 0);
}
