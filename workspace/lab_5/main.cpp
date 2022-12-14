#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <stdio.h>

#include "Ewma.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "BMP280.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

// Application helpers
#include "lora_radio_helper.h"

#define CURRENT_FACTOR 8.43
#define DISABLE_SOLAR_PIN PB_5
#define DISCHARGE_PIN PB_7
#define MAX_NUMBER_OF_EVENTS 10
#define CONFIRMED_MSG_RETRY_COUNTER 3

#define LORA_TIMER 5s

#define ECO_MODE 1
#define HARVEST_MODE 2

#define ACT_MODE_CUR 4.5
#define SLP_MODE_CUR 1.1
#define DSLP_MODE_CUR 0.0045
#define LORA_RX_CUR 10.0
#define LORA_TX_CUR 29.0

#define SDA_PIN PB_9
#define SCL_PIN PB_8
#define SENSOR_ID 0x76

using namespace events;

uint8_t eui [] = MBED_CONF_LORA_DEVICE_EUI;
uint8_t tx_buffer[70];
uint16_t packet_len;
float temp = 0.0;
float pres = 0.0;
short pwr_status = HARVEST_MODE;
chrono::seconds dyn_samp_rate = LORA_TIMER;

// main() runs in its own thread in the OS
AnalogIn solar_voltage(A2);
AnalogIn cap_voltage(A0);

// sensor initialization
I2C* bus = new I2C(SDA_PIN, SCL_PIN);
BMP280* sensor = new BMP280(bus, SENSOR_ID);

// sdcard initialization
SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI,
                 MBED_CONF_SD_SPI_MISO,
                 MBED_CONF_SD_SPI_CLK,
                 MBED_CONF_SD_SPI_CS);
FATFileSystem fs("sd", &sd);

float solar_cur_val;
float cap_vol_val;
float avg_cap_curr_val = 0.0;
float soc;
float node_pwr = 0.0;
float node_curr = 0.0;

float active_time;
float sleep_time;
float deep_sleep_time;

short mode = 0;

Ewma* cap_avg_curr = new Ewma(0.95);
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);
static LoRaWANInterface lorawan(radio);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

/**
 * Connection confiugration
 */
static lorawan_connect_t connection;

static void check_power() {
    mbed_stats_cpu_t stats;
    mbed_stats_cpu_get(&stats);
    active_time = (float) (stats.uptime - stats.sleep_time - stats.deep_sleep_time) / 1000000;
    sleep_time = (float) stats.sleep_time / 1000000;
    deep_sleep_time = (float) stats.deep_sleep_time / 1000000;
    printf("active: %f, sleep: %f, deep sleep: %f", active_time, sleep_time, deep_sleep_time);
    node_curr = ACT_MODE_CUR * active_time + SLP_MODE_CUR * sleep_time + DSLP_MODE_CUR * deep_sleep_time;
    node_pwr = node_curr * 3.3 / 0.8;
    printf("energy: %f J\n", node_pwr);
    switch (pwr_status) {
        case ECO_MODE:
            if (avg_cap_curr_val > 2.0 & soc > 90.0) {
                pwr_status = HARVEST_MODE;
                printf("changed to harvest mode");
            }
            while(node_curr > avg_cap_curr_val * cap_vol_val * 0.8 / 3.3) {
                dyn_samp_rate = dyn_samp_rate + 1s;
            }


            break;
        case HARVEST_MODE:
            if (avg_cap_curr_val < 1.0 || soc < 80.0) {
                pwr_status = ECO_MODE;
                printf("changed to eco mode");
            }
            break;
    }

}

static void read_voltages() {
    solar_cur_val = solar_voltage.read_voltage() / CURRENT_FACTOR * 1000.0;
    cap_vol_val = cap_voltage.read_voltage();
}  

static void send_voltages() {
    soc = (cap_vol_val * cap_vol_val - 0.25) / 7.29 * 100.0;
    printf("Voltage: %.3fV, Current: %.3fmA, StateOfCharge: %.3f%%\n", cap_vol_val, solar_cur_val, soc);
    avg_cap_curr_val = cap_avg_curr->filter(solar_cur_val);
    printf("AverageCurrent: %.3f\n", avg_cap_curr_val);
}

static void read_sensor_data() {
    temp = sensor->getTemperature();
    pres = sensor->getPressure();
    packet_len = sprintf((char *) tx_buffer, "Temperature: %f gradC, Pressure %f hPa", temp, pres);
    //printf("Read %d bytes of data\r\n", packet_len);
    printf("%s\n", (char *) tx_buffer);
}

static void send_lora_message() {
    
    int16_t retcode;

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len, MSG_UNCONFIRMED_FLAG);

    printf("retcode: %d", retcode);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
        : printf("\r\n send() - Error code %d \r\n", retcode);
        return;
    }

    printf("\r\n %d bytes scheduled for transmission... \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
    printf("After memset");
}

int configLora() {
      // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }
    printf("\r\n Mbed LoRaWANStack initialized \r\n");
    
    // print the device EUI
    printf("\r\n Device EUI: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x \r\n",
           eui[0], eui[1], eui[2], eui[3], eui[4], eui[5], eui[6], eui[7]);

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
            != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adative_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    retcode = lorawan.connect(); //orig

    if (retcode == LORAWAN_STATUS_OK ||
            retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    return 0;
}

int main()
{   
    fs.mount(&sd);
    sensor->wakeUp();

    configLora();
    solar_voltage.set_reference_voltage(3.3);
    cap_voltage.set_reference_voltage(3.3);
    ev_queue.call_every(1s, read_voltages);
    ev_queue.call_every(2s, send_voltages);
    ev_queue.call_every(1s, read_sensor_data);
    ev_queue.call_every(10s, check_power);
    ev_queue.dispatch_forever();
    /*while (1) {
        solar_cur_val = solar_voltage.read_voltage() / CURRENT_FACTOR * 1000.0;
        cap_vol_val = cap_voltage.read_voltage();
        soc = (cap_vol_val * cap_vol_val - 0.25) / 7.29 * 100.0;
        printf("Voltage: %.3fV, Current: %.3fmA, StateOfCharge: %.3f%%\n", cap_vol_val, solar_cur_val, soc);
        //printf("test\n");
        ThisThread::sleep_for(1s);
    


        switch (mode) {
            case HARVEST_MODE:
                break;
            case ECO_MODE:
                break;
        }
    }*/
}


/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{

    printf("\r\nEvent: %d\r\n", event);
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            ev_queue.call_every(LORA_TIMER, send_lora_message);
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_lora_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            //receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_lora_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}
