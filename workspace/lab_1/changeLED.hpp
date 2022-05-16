#include "ThisThread.h"
#include "DigitalOut.h"
#include "mbed.h"

#define TEMPO_SLOW 250ms
#define TEMPO_FAST 50ms



class blinkLED {
    DigitalOut led(LED1);
    static volatile bool led_state = false;
};


void blinkLED::speedChange(void) {
    led_state = !led_state;
}

int main() {
    while (true) {
        if(led_state) {
            ThisThread::sleep_for(TEMPO_FAST);
        }
        else {
            ThisThread::sleep_for(TEMPO_SLOW);
        }     

        led = !led;
    }
}