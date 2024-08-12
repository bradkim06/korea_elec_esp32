#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32
#include "esp32/rom/uart.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/uart.h"
#endif

#define EX_UART_PC UART_NUM_0
#define EX_UART_MODULE UART_NUM_2
#define EX_UART_LORA UART_NUM_1

#define TXD_SENSOR_PIN (GPIO_NUM_4)
#define RXD_SENSOR_PIN (GPIO_NUM_16)
// #define TXD_LORA_PIN (GPIO_NUM_26)
// #define RXD_LORA_PIN (GPIO_NUM_25)
#define TXD_LORA_PIN (GPIO_NUM_25)
#define RXD_LORA_PIN (GPIO_NUM_26)
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define P2_PARSING_SYNC 0
#define P2_PARSING_VERSION 1
#define P2_PARSING_COUNT 2
#define P2_PARSING_DATA 3
#define P2_PARSING_SWITCH 4
#define P2_PARSING_FOOTER 5
#define P2_12b_PROTOCOL 1

#define RAW_DATA_POS_MAX 100
#define SIZE_BUFF_RECEIVER 14

class Uart {
   public:
    void PcUartinit();
    void Uart1init();
    void Uart2init();
    void Uart_Send(int uart_num, char *msg, int length);
    void Uart2Send();
    int DataExistCheck(int raw_data_size_output);

   private:
    void handleMessage();
};
