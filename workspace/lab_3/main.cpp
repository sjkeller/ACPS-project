
#include "mbed.h"
#include "Sensor.h"

// main() runs in its own thread in the OS
int main()
{
    const PinName sdaPin = PB_9;
    const PinName scaPin = PB_8;
    const int bme280Add = 0x76;
    //BME280* sensor = new BME280(sdaPin, scaPin, bme280Add);
    BME280 sensor(sdaPin, scaPin, bme280Add);
    sensor.wakeUp();
    while (true) {
        //sensor->wakeUp();
        //int32_t* data = sensor->readSensorData();
        //int32_t data = *dataPointer;
        printf("test");
        //sensor->sleep();
    }
}

