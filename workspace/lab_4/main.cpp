#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers
#include "DummySensor.h"
#include "BMP280.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"

#include "SDBlockDevice.h"
#include "FATFileSystem.h"
//#include "SDFileSystem.h"

using namespace events;
// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<35 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[70];
uint8_t rx_buffer[70];

// Variable to store the DEV-EUI for this board
uint8_t eui [] = MBED_CONF_LORA_DEVICE_EUI; // (obsolete), retrieve EUI value that was set via mbed_app.json

const PinName sdaPin = PB_9;
const PinName sclPin = PB_8;
const int bme280Add = 0x76;

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
// #define TX_TIMER                        4000ms
#define READ_TIMER 200ms
#define UART_TIMER 1s
#define SD_TIMER 5s
#define LORA_TIMER 5s

/*
 * Waiting time until retry of a failed transmission
 */
#define RETRY_INTERVAL                  3000ms

/*
 * Instantiate an LED used for transmission indication
 */
DigitalOut led(LED1);

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
 * Dummy pin for dummy sensor
 */
#define PC_9                            0

/**
 * Dummy sensor class object
 */

I2C* bus = new I2C(sdaPin, sclPin);

BMP280* sensor = new BMP280(bus, bme280Add);
DS1820  ds1820(PC_9);

/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

/**
 * Connection confiugration
 */
static lorawan_connect_t connection;

int ev_log_id;
int ev_sensor_id;
int ev_lora_id;
int ev_uart_id;
int ev_lora_receive_id;
bool ev_log_closed = 0;
bool ev_sensor_closed = 0;
bool ev_lora_closed = 0;
bool ev_uart_closed = 0;

void formatSD() {
    //mount SD-Card
    SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI,
                 MBED_CONF_SD_SPI_MISO,
                 MBED_CONF_SD_SPI_CLK,
                 MBED_CONF_SD_SPI_CS);
    FATFileSystem fs("sd", &sd);
    const int mounted = fs.mount(&sd);
    printf("Mounted with code %d", mounted);
    printf("starting format ... ");
    fflush(stdout);
    fs.format(&sd);
    printf("formatting done\n");
    fflush(stdout);
    /*
    //Generate a new log file with the correct name and header
    time_t live_time = time(NULL);
    printf("\n live time: %u", live_time);
    char path_buffer[50] = {0};
    strftime(path_buffer, 50, "/sd/%F_%H-%M-%S.log", localtime(&live_time));
    printf("\n%s", path_buffer);

    FILE *fp = fopen(path_buffer, "w+");
    */
    char path_buffer[2] = "a";
    //FILE *fp = fopen(path_buffer, "w+");
    printf("starting log write\n");
    //fprintf(fp, "Time (s), Pressure (hPa), Temperature (degC)\n");
    //fclose(fp);
}
/**
 * Entry point for application
 */
int main(void)
{
    //formatSD();
    // setup tracing
    setup_trace();
    sensor->wakeUp();

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
        printf("\r\n enable_adaptive_datarate failed! \r\n");
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

    ev_queue.dispatch_forever();

    return 0;
}

float temp = 0.0;
float pres = 0.0;

uint16_t packet_len;
/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    if (ev_sensor_closed) {
        packet_len = sprintf((char *) tx_buffer, "Temperature: %f gradC, Pressure %f hPa", temp, pres);
    }
    
    int16_t retcode;

    led = !led; //toggle LED

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);

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
static void send_empty_message()
{
    packet_len = 0;
    
    int16_t retcode;

    led = !led; //toggle LED

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);

    printf("retcode: %d", retcode);

    if (retcode < 0) { // Failed to send message (duty-cycle violation?
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
        : printf("\r\n send() - Error code %d \r\n", retcode);
        return;
    }

    printf("\r\n %d bytes scheduled for transmission... \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
    printf("After memset");
}

/*!
 * Reads Data from sensor and writes it to the transmission buffer
 */

void readData() {
    temp = sensor->getTemperature();
    pres = sensor->getPressure();
    packet_len = sprintf((char *) tx_buffer, "Temperature: %f gradC, Pressure %f hPa", temp, pres);
    printf("Read %d bytes of data\r\n", packet_len);
}

/*!
 * Prints sensor data to uart
 */

void printUART() {
    printf("Temperature: %f gradC, Pressure %f hPa\r\n", temp, pres);
}

/*!
 * Saves sensor data to SD card
 */

void saveSD() {
/*
    time_t live_time = time(NULL);
    strftime(path_buffer, 50, "/sd/%F_%H-%M-%S.log", localtime(&live_time));
    FILE *fp = fopen(path_buffer, "w+");
    fprintf(fp, "Time (s), Pressure (hPa), Temperature (degC)\n");

    // print data to file
    fprintf(fp, "%s, %d.%d, %d.%d\n", logtime_buffer, real_pres / 100, real_pres % 100, real_temp / 100, real_temp % 100);
    printf("\n%s, %d.%d, %d.%d\n", logtime_buffer, real_pres / 100, real_pres % 100, real_temp / 100, real_temp % 100);
    fclose(fp);
*/
}
/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);
    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        // printf("%02x ", rx_buffer[i]); //print received bytes as uint8 values
        printf("%c", (char)rx_buffer[i]); //convert bytes to char and print the received msg
    }
    printf("\r\n");
    
    // Sensor measurment control
    if (!strcmp((const char*)rx_buffer, "sensor start")) {
        if(!ev_sensor_closed) {
            printf("sensor measurement already running!\r\n");
        }
        else {
            ev_sensor_id = ev_queue.call_every(READ_TIMER, readData);
            ev_sensor_closed = 0;
            printf("sensor measurement started\r\n");
        }
    }
    else if (!strcmp((const char*)rx_buffer, "sensor stop")) {
        ev_sensor_closed = ev_queue.cancel(ev_sensor_id);
        if (ev_sensor_closed) {
            printf("sensor measurement stopped\r\n");
        }
        else {
            printf("sensor measurment couldn't be stopped!");
            ev_sensor_closed = 1;
        }
    }

    // data logging control
    else if (!strcmp((const char*)rx_buffer, "log start")) {
        if(!ev_log_closed) {
            printf("logging already running!\r\n");
        }
        else {
            ev_log_id = ev_queue.call_every(SD_TIMER, saveSD);
            ev_log_closed = 0;
            printf("data logging started\r\n");
        }
    }
    else if (!strcmp((const char*)rx_buffer, "log stop")) {
        ev_log_closed = ev_queue.cancel(ev_log_id);
        if (ev_log_closed)
            printf("data logging stopped\r\n");
        else {
            printf("data logging couldn't be stopped!\r\n");
            ev_log_closed = 1;
        }
    }

    // lora control
    else if (!strcmp((const char*)rx_buffer, "lora start")) {
        if(!ev_lora_closed) {
            printf("lora already running!\r\n");
        }
        else {
            ev_queue.cancel(ev_lora_receive_id);
            ev_lora_id = ev_queue.call_every(LORA_TIMER, send_message);
            ev_lora_closed = 0;
            printf("lora started\r\n");
        }
    }
    else if (!strcmp((const char*)rx_buffer, "lora stop")) {
        ev_lora_closed = ev_queue.cancel(ev_lora_id);
        if (ev_lora_closed) {
            printf("lora stopped\r\n");
            ev_lora_receive_id = ev_queue.call_every(LORA_TIMER, send_empty_message);
        }
        else {
            printf("lora couldn't be stopped!\r\n");
            ev_log_closed = 1;
        }
    }

    // UART control
    else if (!strcmp((const char*)rx_buffer, "uart start")) {
        if(!ev_uart_closed) {
            printf("uart already running!\r\n");
        }
        else {
            ev_uart_id = ev_queue.call_every(UART_TIMER, printUART);
            ev_uart_closed = 0;
            printf("uart started\r\n");
        }
    }
    else if (!strcmp((const char*)rx_buffer, "uart stop")) {
        ev_uart_closed = ev_queue.cancel(ev_uart_id);
        if (ev_uart_closed)
            printf("uart stopped\r\n");
        else {
            printf("uart couldn't be stopped!\r\n");
            ev_uart_closed = 1;
        }
    }   

    memset(rx_buffer, 0, sizeof(rx_buffer));
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
            ev_sensor_id = ev_queue.call_every(READ_TIMER, readData);
            ev_lora_id = ev_queue.call_every(LORA_TIMER, send_message);
            ev_uart_id = ev_queue.call_every(UART_TIMER, printUART);
            ev_log_id = ev_queue.call_every(SD_TIMER, saveSD);
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
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
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
                send_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
