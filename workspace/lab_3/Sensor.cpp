#include "Sensor.h"

BME280::BME280(const PinName sdaPin, const PinName sclPin, const int bme280Add) : sensorAdd(bme280Add) {
    I2C busTemp(sdaPin, sclPin);
    bus = &busTemp;

    // get calibration data
    constexpr int calibrationRegister = 0x88;
    constexpr int calibRegLen = 24;
    readBMP(calibrationRegister, calib_data, calibRegLen);

    // get calibration contants
    dig_T1 = getCalib(calib_data, 0, true);
    dig_T2 = getCalib(calib_data, 2);
    dig_T3 = getCalib(calib_data, 4);
    dig_P1 = getCalib(calib_data, 6, true);
    dig_P2 = getCalib(calib_data, 8);
    dig_P3 = getCalib(calib_data, 10);
    dig_P4 = getCalib(calib_data, 12);
    dig_P5 = getCalib(calib_data, 14);
    dig_P6 = getCalib(calib_data, 16);
    dig_P7 = getCalib(calib_data, 18);
    dig_P8 = getCalib(calib_data, 20);
    dig_P9 = getCalib(calib_data, 22);
};

void BME280::wakeUp() {
    constexpr int wakeUpValue = (7 << 5) + (7 << 2) + 3;
    //writeBMP(0xF4, (7 << 5) + (7 << 2) + 3);
    //writeBMP(modeRegister, (7 << 5) + (7 << 2) + 3);
    writeBMP(modeRegister, wakeUpValue);
    //Wait for wake up
    ThisThread::sleep_for(200ms);
}

void BME280::sleep() {
    constexpr int sleepValue = (7 << 5) + (7 << 2);
    writeBMP(modeRegister, sleepValue);
}

/*! *  Reads from the BMP280 via I2C
 *  \param address  The address to read from
 *  \param data     The buffer to which the values will be read
 *  \param n        Number of bytes to read
 *  \return         Returns 0 on success
 */
int BME280::readBMP(char address, char* data, int n) {
    int succ_write, succ_read;
    succ_write = bus->write(sensorAdd << 1, &address, 1);
    succ_read = bus->read((sensorAdd << 1) + 1, data, n);
    return succ_write + succ_read;
}

/*!
 *  Writes to the BMP280 via I2C
 *  \param address  The address to write to
 *  \param value    The value to be written
 */
int BME280::writeBMP(char address, char value) {
    char write_array[2];
    write_array[0] = address;
    write_array[1] = value;
    int returnValue = bus->write(sensorAdd << 1, write_array, 2);
    /*
    address = 0x88;
    int returnValue = bus->write(sensorAdd << 1, &address, 1);
    */
    return returnValue;
}

/*!
 *  Gets BMP280 calibration data from the given data
 *  \param data  The data, which contains the calibration data
 *  \param data  The index of the wanted calibration data
 *  \param n     If the calibration data is in unsigned format
 *  \return      The wanted calibration data
 */
int BME280::getCalib(char* data, int index, bool u){
    if (u)
        return (unsigned short) ((short) data[index] + ((short) data[index + 1] << 8));
    return ((short) data[index] + ((short) data[index + 1] << 8));
}

int32_t* BME280::readSensorData(void) {
    // get raw register data
    char data_buffer[8];
    constexpr int dataAdress = 0xF7;
    constexpr int readLength = 6;
    readBMP(dataAdress, data_buffer, readLength);

    // construct raw data from registers
    pressure = ((int32_t) data_buffer[0] << 12) + ((int32_t) data_buffer[1] << 4) + ((int32_t) data_buffer[2] >> 4);
    temperature = ((int32_t) data_buffer[3] << 12) + ((int32_t) data_buffer[4] << 4) + ((int32_t) data_buffer[5] >> 4);

    // convert raw data to physical values
    const int32_t real_temp = calc_temp(temperature);
    const int32_t real_pres = calc_press(pressure);
    int32_t tempAndPres[2];
    tempAndPres[0] = real_pres;
    tempAndPres[1] = real_temp;
    return tempAndPres;
}

/*!
 *  Calculates the temperature in °C from the given raw temperature
 *  \param raw_temp The temperature measurement from the raw data
 */
int32_t BME280::calc_temp(int32_t raw_temp){
    int32_t temp;
    // Integer calculations
    temp_fine = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    temp_fine += (((((raw_temp >> 4) - ((int32_t)dig_T1)) * ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    temp = ((temp_fine * 5) + 128) >> 8;
    return temp;
}

/*!
 *  Calculates the pressure in °C from the given raw pressure
 *  \param raw_press The pressure measurement from the raw data
 */
uint32_t BME280::calc_press(int32_t raw_press)
{
    uint32_t p;
    // 32-bit only calculations
    int32_t var1, var2;

    var1 = (((int32_t)temp_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = (((32768 + var1)) * ((int32_t)dig_P1)) >> 15;
    if (var1 == 0) {
      // avoid exception caused by division by zero
      return 0;
    }
    p = (((uint32_t)(((int32_t)1048576) - raw_press) - (uint32_t)(var2 >> 12))) * 3125U;
    if (p < 0x80000000U) {
      p = (p << 1) / ((uint32_t)var1);
    }
    else {
      p = (p / (uint32_t)var1) << 1;
    }
    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
    //   p = p * 1000U; // convert to mPa
    return p;
}