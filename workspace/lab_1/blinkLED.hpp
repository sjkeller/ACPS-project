#include "ThisThread.h"
#include "DigitalOut.h"
#include "mbed.h"

#define BLINKING_RATE 1s

class blinkLED {
    DigitalOut led_1(LED1);
    DigitalOut led_3(LED3);
    DigitalOut led_4(LED4);
};

int main() {
    while (true) {
        led_1 = !led_1;
        led_3 = !led_3;
        led_4 = !led_4;
        ThisThread::sleep_for(BLINKING_RATE);
        printf("LED 1 state: %d\n", (int) led_1);
        printf("LED 3 state: %d\n", (int) led_3);
        printf("LED 4 state: %d\n", (int) led_4);
    }
}