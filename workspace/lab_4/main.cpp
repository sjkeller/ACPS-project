#include "mbed.h"
#include "BMP280.h"

const PinName sdaPin = PB_9;
const PinName sclPin = PB_8;
I2C* bus = new I2C(sdaPin, sclPin);
const int bme280Add = 0x76;
BMP280* sensor = new BMP280(bus, sensor);

float temp;
float press;

void readData(BMP280* sensor) {
    sensor->wakeUp();
    temp = sensor->getTemperature();
    press = sensor->getPressure();
}

void writeSd() {

}

void writeUart() {

}

void writeLora() {

}

EventQueue queue;
int pending;
enum state {READ, SD, UART, LORA};

void stateMachine(state activeState) {
    switch (activeState) {
        case READ:
            readData(sensor);
            pending = queue.call(&stateMachine, SD);
            break;
        case SD:
            writeSd();
            queue.call_in(5000ms, &stateMachine, READ);
            break;
        case UART:
            writeUart();
            pending = queue.call(&stateMachine, LORA);
            break;
        case LORA:
            writeLora();
            queue.call_in(1000ms, &stateMachine, UART);
            pending = queue.call_in(200ms, &stateMachine, LORA);
            break;
    }
}

int main (void) {
    // queue.cancel(pending); // can be used to cancel pending events
    queue.call(&stateMachine, READ); //start state machine
    // events are executed by the dispatch method
    queue.dispatch_forever();
    delete bus;
    delete sensor;
}