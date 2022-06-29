#include "mbed.h"
//#include "embed_stats.h"

#define BLINK_FREQ 200ms

LowPowerTicker flippy;
//Ticker flippy;
EventQueue stats_queue;
DigitalOut myled_1(LED1);
DigitalOut myled_2(LED2);
DigitalOut myled_3(LED3);
DigitalOut myled_4(LED4);

void flip_led() {
    myled_1 = !myled_1;
    myled_2 = !myled_2;
    myled_3 = !myled_3;
    myled_4 = !myled_4;

}

void get_stats() {
    mbed_stats_cpu_t stats;
    mbed_stats_cpu_get(&stats);
    printf("UPTIME: %llu\n", stats.uptime / 1000);
    printf("SLEEP: %llu\n", stats.sleep_time / 1000);
    printf("DEEPSLEEP: %llu\n\n", stats.deep_sleep_time / 1000);
}

// main() runs in its own thread in the OS
int main()
{
    myled_1 = 1;
    myled_2 = 1;
    myled_3 = 1;
    myled_4 = 1;



    stats_queue.call_every(1s, get_stats);

    flippy.attach(&flip_led, BLINK_FREQ);

    stats_queue.dispatch_forever();
    while (true) {



        ThisThread::sleep_for(1s);
    }
}

