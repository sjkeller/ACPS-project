#include "mbed.h"
#include "MovingAverage.h"

#define CURRENT_FACTOR 8.43
#define DISABLE_SOLAR_PIN PB_5
#define DISCHARGE_PIN PB_7
#define MAX_NUMBER_OF_EVENTS 3

#define ECO_MODE 1
#define HARVEST_MODE 2

#define ACT_MODE_CUR 4.5
#define SLP_MODE_CUR 1.1
#define LORA_RX_CUR 10.0
#define LORA_TX_CUR 29.0

// main() runs in its own thread in the OS
AnalogIn solar_voltage(A2);
AnalogIn cap_voltage(A0);

float solar_cur_val;
float cap_vol_val;
float soc;

short mode = 0;
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

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

int main()
{
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

