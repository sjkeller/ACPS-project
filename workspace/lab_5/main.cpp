#include "mbed.h"
#include "MovingAverage.h"
#include <cstdio>
#include <cstring>
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers
#include "lora_radio_helper.h"

#define CURRENT_FACTOR 8.43
#define DISABLE_SOLAR_PIN PB_5
#define DISCHARGE_PIN PB_7
#define MAX_NUMBER_OF_EVENTS 10
#define CONFIRMED_MSG_RETRY_COUNTER 3

#define ECO_MODE 1
#define HARVEST_MODE 2

#define ACT_MODE_CUR 4.5
#define SLP_MODE_CUR 1.1
#define LORA_RX_CUR 10.0
#define LORA_TX_CUR 29.0
using namespace events;

uint8_t eui [] = MBED_CONF_LORA_DEVICE_EUI;
// main() runs in its own thread in the OS
AnalogIn solar_voltage(A2);
AnalogIn cap_voltage(A0);

float solar_cur_val;
float cap_vol_val;
float soc;

short mode = 0;

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


MovingAverage* cap_avg_curr = new MovingAverage(10);

void readVoltages() {
    solar_cur_val = solar_voltage.read_voltage() / CURRENT_FACTOR * 1000.0;
    cap_vol_val = cap_voltage.read_voltage();
    cap_avg_curr->add(solar_cur_val);
}  

void sendVoltages() {
    soc = (cap_vol_val * cap_vol_val - 0.25) / 7.29 * 100.0;
    printf("Voltage: %.3fV, Current: %.3fmA, StateOfCharge: %.3f%%\n", cap_vol_val, solar_cur_val, soc);
    printf("AverageCurrent: %.3f\n", cap_avg_curr->getCurrentAverage());
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

    configLora();
    solar_voltage.set_reference_voltage(3.3);
    cap_voltage.set_reference_voltage(3.3);
    ev_queue.call_every(1s, readVoltages);
    ev_queue.call_every(2s, sendVoltages);
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
            // start all event queue calls
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
                //send_message();
                printf("sneed seeds and feeds");
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            //receive_message();
            printf("sneed seeds and feeds");
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
                //send_message();
                printf("sneed seeds and feeds");
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}
