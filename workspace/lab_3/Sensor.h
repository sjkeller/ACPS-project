//#ifndef BME280
//#define BME280

#include "mbed.h"

class BME280 {
    public:
        BME280(const PinName sdaPin, const PinName sclPin, const int bme280Add);

        void wakeUp();
        void sleep();
        int32_t* readSensorData();

    private:
        int readBMP(char address, char* data, int n = 1);
        int writeBMP(char address, char value);
        int getCalib(char* data, int index, bool u = false);
        int32_t calc_temp(int32_t raw_temp);
        uint32_t calc_press(int32_t raw_press);
        int sensorAdd = 0;
        I2C* bus;
        const int modeRegister = 0xF4;
        char calib_data[26];
        unsigned short dig_T1 = 0;
        short dig_T2 = 0;
        short dig_T3 = 0;
        unsigned short dig_P1 = 0;
        short dig_P2 = 0;
        short dig_P3 = 0;
        short dig_P4 = 0;
        short dig_P5 = 0;
        short dig_P6 = 0;
        short dig_P7 = 0;
        short dig_P8 = 0;
        short dig_P9 = 0;
        int32_t temp_fine = 0.0;
        int32_t temperature = 0;
        int32_t pressure = 0;
};

//#endif