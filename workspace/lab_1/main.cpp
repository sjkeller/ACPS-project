#include "DigitalIn.h"
#include "InterruptIn.h"
#include "ThisThread.h"
#include "mbed.h"
#include <iostream>

#define TEMPO_SLOW 250ms
#define TEMPO_FAST 50ms

static volatile bool led_state = false;
// main() runs in its own thread in the OS
void speedChange(void) {
    led_state = !led_state;
}


int main()
{
    DigitalOut led_one(LED1);
    DigitalOut led_thr(LED3);
    DigitalOut led_fou(LED4);


    AnalogIn analog_input(PA_5);



    while (true) {
        printf("Analog output %d\n", (int) (100.0 * analog_input.read()));
        ThisThread::sleep_for(100ms);
    }
}

