#include "Uart.h"

#include <cstdio>
#include <cstring>
#include <string>

#define PATTERN_CHR_NUM                                                      \
    (3) /*!< Set the number of consecutive and identical characters received \
           by receiver which defines a UART pattern*/

intr_handle_t handle_console;
static const char *TAG = "uart_events";
uint16_t urxlen;

int protocol_count_err;
int protocol_over_flow_err;
int protocol_sec;

int pre_protocol_count_err = 0;
int pre_protocol_over_flow_err = 0;
int pre_protocol_sec = 0;

int pre_count_no = -1;
int parsing_length;
int parsing_datapos;

uint8_t parsing_data[33];
int parsing_state = P2_PARSING_FOOTER;
int raw_data_in_pos = 0;

extern uint8_t raw_data[RAW_DATA_POS_MAX][31];  // 6*2ch, switch, packet_no
extern uint8_t raw_data_size[RAW_DATA_POS_MAX];

static QueueHandle_t uart0_queue;
static void uart2_rx_task(void *arg);

static uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 0,
    .source_clk = UART_SCLK_DEFAULT,
};

void Uart::PcUartinit() {
    ESP_ERROR_CHECK(uart_param_config(EX_UART_PC, &uart_config));

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Set UART pins (using UART0 default pins ie no changes.)
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_PC, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install UART driver, and get the queue.
    // ESP_ERROR_CHECK(
    uart_driver_install(EX_UART_PC, BUF_SIZE * 2, 0, 0, NULL, 0);
}

void Uart::Uart1init() {
    ESP_ERROR_CHECK(uart_driver_install(EX_UART_LORA, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(EX_UART_LORA, &uart_config));

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Uart Lora Init");
    // Set UART pins (using UART0 default pins ie no changes.)
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_LORA, TXD_LORA_PIN, RXD_LORA_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install UART driver, and get the queue.

}

void Uart::Uart2init() {
    // Install UART driver, and get the queue.
    ESP_ERROR_CHECK(uart_driver_install(EX_UART_MODULE, BUF_SIZE * 10, 0, 20,
                                        &uart0_queue, 0));

    ESP_ERROR_CHECK(uart_param_config(EX_UART_MODULE, &uart_config));

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Set UART pins (using UART0 default pins ie no changes.)
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_MODULE, TXD_SENSOR_PIN, RXD_SENSOR_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_MODULE, '+', PATTERN_CHR_NUM, 9,
                                      0, 0);

    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_MODULE, 20);

    // // enable RX interrupt
    // ESP_ERROR_CHECK(uart_enable_rx_intr(EX_UART_MODULE));

    xTaskCreatePinnedToCore(uart2_rx_task, "uart_rx_task", 1024 * 10, NULL, 1, NULL, 0);
}

static void uart2_intr_handle(size_t rx_len, uint8_t *dtmp) {
    static const char *HANDLER_TAG = "RX_HANDLER";
    esp_log_level_set(HANDLER_TAG, ESP_LOG_INFO);

    for (int i = 0; i < rx_len; i++) {
        int value = *(dtmp + i);
        //    Serial.print(value,HEX);Serial.print(",");
        switch (parsing_state) {
            case P2_PARSING_FOOTER:
                if ((value & 0xFF) == 0xA5) parsing_state = P2_PARSING_SYNC;
                break;
            case P2_PARSING_SYNC:
                if ((value & 0xFF) == 0x5A)
                    parsing_state = P2_PARSING_VERSION;
                else
                    parsing_state = P2_PARSING_FOOTER;
                break;
            case P2_PARSING_VERSION:
                if ((value & 0xFF) == 0x02)
                    parsing_state = P2_PARSING_COUNT;
                else
                    parsing_state = P2_PARSING_FOOTER;
                break;
            case P2_PARSING_COUNT:
                if (pre_count_no >= 0) {
                    pre_count_no++;
                    if (pre_count_no >= 256) {
                        pre_count_no = 0;
                        protocol_sec++;
                    }
                    if ((int)(value & 0xFF) != pre_count_no) {
                        protocol_count_err++;
                        pre_count_no = -1;
                        parsing_state = P2_PARSING_FOOTER;
                        break;
                    }
                }
                pre_count_no = (int)(value & 0xFF);

                parsing_length = 12;
                parsing_datapos = 0;
                parsing_state = P2_PARSING_DATA;
                break;
            case P2_PARSING_DATA:
                parsing_data[parsing_datapos] = value;
                parsing_datapos++;
                if (parsing_datapos >= parsing_length)
                    parsing_state = P2_PARSING_SWITCH;
                break;

            case P2_PARSING_SWITCH:
                parsing_data[parsing_datapos] = value;
                parsing_datapos++;
                parsing_data[parsing_datapos] = pre_count_no;
                if (raw_data_size[raw_data_in_pos] > 0) {
                    //  Serial.print("raw_data exist already at :
                    //  ");Serial.println(raw_data_in_pos);
                    protocol_over_flow_err++;
                }
                //              *raw_data = parsing_data;
                for (int j = 0; j < SIZE_BUFF_RECEIVER; j++) {
                    raw_data[raw_data_in_pos][j] = parsing_data[j];
                }
                raw_data_size[raw_data_in_pos] = 14;
                raw_data_in_pos++;
                if (raw_data_in_pos >= RAW_DATA_POS_MAX) {
                    raw_data_in_pos = 0;
                }
                parsing_state = P2_PARSING_FOOTER;
                break;
        }
    }
    // after reading bytes from buffer clear UART interrupt status
    uart_clear_intr_status(EX_UART_MODULE,
                           UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
}

static void uart2_rx_task(void *arg) {
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

    uart_event_t event;
    size_t buffered_size;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);

    while (1) {
        if (xQueueReceive(uart0_queue, (void *)&event,
                          (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", EX_UART_MODULE);
            switch (event.type) {
                // Event of UART receving data
                /*We'd better handler data event fast, there would be much more
                data events than other types of events. If we take too much time
                on data event, the queue might be full.*/
                case UART_DATA: {
                    uart_read_bytes(EX_UART_MODULE, dtmp, event.size,
                                    portMAX_DELAY);
                    uart2_intr_handle(event.size, dtmp);
                    break;
                }
                // Event of HW FIFO overflow detected
                case UART_FIFO_OVF: {
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding
                    // flow control for your application. The ISR has already
                    // reset the rx FIFO, As an example, we directly flush the
                    // rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_MODULE);
                    xQueueReset(uart0_queue);
                    break;
                }
                // Event of UART ring buffer full
                case UART_BUFFER_FULL: {
                    ESP_LOGI(TAG, "ring buffer full");

                    // If buffer full happened, you should consider encreasing
                    // your buffer size As an example, we directly flush the rx
                    // buffer here in order to read more data.
                    uart_flush_input(EX_UART_MODULE);
                    xQueueReset(uart0_queue);
                    break;
                }
                // Event of UART RX break detected
                case UART_BREAK: {
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                }
                // Event of UART parity check error
                case UART_PARITY_ERR: {
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                }
                // Event of UART frame error
                case UART_FRAME_ERR: {
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                }
                // UART_PATTERN_DET
                case UART_PATTERN_DET: {
                    uart_get_buffered_data_len(EX_UART_MODULE, &buffered_size);
                    int pos = uart_pattern_pop_pos(EX_UART_MODULE);
                    ESP_LOGI(
                        TAG,
                        "[UART PATTERN DETECTED] pos: %d, buffered size: %d",
                        pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the
                        // pattern position queue is full so that it can not
                        // record the position. We should set a larger queue
                        // size. As an example, we directly flush the rx buffer
                        // here.
                        uart_flush_input(EX_UART_MODULE);
                    } else {
                        uart_read_bytes(EX_UART_MODULE, dtmp, pos,
                                        100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(EX_UART_MODULE, pat, PATTERN_CHR_NUM,
                                        100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "read pat : %s", pat);
                    }
                    break;
                }
                // Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }

    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

int Uart::DataExistCheck(int raw_data_size_output) {
    int returnData = 0;
    if (raw_data_size_output > 0) {
        returnData = 1;
    } else {
        if (protocol_count_err != pre_protocol_count_err) {
            handleMessage();
        }
        if (protocol_over_flow_err != pre_protocol_over_flow_err) {
            handleMessage();
        }
        if (protocol_sec != pre_protocol_sec) {
            handleMessage();
        }
    }
    return returnData;
}
void Uart::Uart_Send(int uart_num, char *msg, int length) {
    uart_write_bytes(uart_num, msg, length);
}

void Uart::Uart2Send() {}

void Uart::handleMessage() {
    if (protocol_count_err != pre_protocol_count_err) {
        pre_protocol_count_err = protocol_count_err;
    } else if (protocol_over_flow_err != pre_protocol_over_flow_err) {
        pre_protocol_over_flow_err = protocol_over_flow_err;
    } else if (protocol_sec != pre_protocol_sec) {
        if ((protocol_sec % 10) == 0) {
        }
        pre_protocol_sec = protocol_sec;
    }
}
