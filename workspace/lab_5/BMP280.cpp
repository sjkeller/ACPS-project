#include "BMP280.h"

BMP280::BMP280(I2C* i2c_bus, int address){
    i2c = i2c_bus;
    address_8bit = address << 1;
    getCalibrationParameters();
}

void BMP280::wakeUp(void){
    writeRegister8Bit(BMP280_REGISTER_CONTROL, 0xFF); // ctrl_meas = normale mode and full oversampling
} 

void BMP280::writeRegister8Bit(char reg, char data){
    char buffer[2]; // Two bytes to send during write operation, reg and data
    buffer[0] = reg; // Address of register being accessed
    buffer[1] = data;    // data to write to the register
    printf("ack = %d\n",i2c->write(address_8bit, buffer, 2)); // ack = 0 => OK
}

void BMP280::getCalibrationParameters(){
    cali.dig_T1 = readRegister16Bit_LSB(BMP280_REGISTER_DIG_T1);    
    cali.dig_T2 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_T2);
    cali.dig_T3 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_T3);
    cali.dig_P1 = readRegister16Bit_LSB(BMP280_REGISTER_DIG_P1);
    cali.dig_P2 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P2);
    cali.dig_P3 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P3);
    cali.dig_P4 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P4);
    cali.dig_P5 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P5);
    cali.dig_P6 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P6);
    cali.dig_P7 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P7);
    cali.dig_P8 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P8);
    cali.dig_P9 = (int16_t) readRegister16Bit_LSB(BMP280_REGISTER_DIG_P9);

    printf("dig_T1: %d \n", cali.dig_T1);
    printf("dig_T2: %d \n", cali.dig_T2);
    printf("dig_T3: %d \n", cali.dig_T3);
}

float BMP280::getTemperature(){
    int32_t adc_T = readRegister20Bit(BMP280_REGISTER_TEMPDATA);
    //printf("adc Temp: %d \n", adc_T);

    /*seite 22*/
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)cali.dig_T1 << 1))) * ((int32_t)cali.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)cali.dig_T1)) * ((adc_T >> 4) - ((int32_t)cali.dig_T1))) >> 12) * ((int32_t)cali.dig_T3)) >> 14;

    fineTemperature = (var1 + var2);

    T = (fineTemperature * 5 + 128) >> 8;
    return (float)T / 100;
}


float BMP280::getPressure(){    
    getTemperature();
    int32_t adc_P = readRegister20Bit(BMP280_REGISTER_PRESSUREDATA);

    /*seite 22*/

    int64_t var1, var2, p;

    var1 = ((int64_t)fineTemperature) - 128000;  ///???????? fineTemperature>>1
    var2 = var1 * var1 * (int64_t)cali.dig_P6;
    var2 = var2 + ((var1 * (int64_t)cali.dig_P5) << 17);
    var2 = var2 + (((int64_t)cali.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)cali.dig_P3) >> 8) + ((var1 * (int64_t)cali.dig_P2) << 12);
    var1 =(((((int64_t)1) << 47) + var1)) * ((int64_t)cali.dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)cali.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)cali.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)cali.dig_P7) << 4);

    return (float) p / 256.0 /100.0;
}

uint16_t BMP280::readRegister16Bit_LSB(char reg) {
    char buffer[2];
    i2c->write(address_8bit, &reg, 1);
    i2c->read(address_8bit, buffer, 2);
    return uint16_t(buffer[1]) << 8 | uint16_t(buffer[0]);
}

uint32_t BMP280::readRegister20Bit(char reg) {
    char buffer[3];
    i2c->write(address_8bit, &reg, 1);
    i2c->read(address_8bit, buffer, 3);
    return uint32_t(buffer[0]) << 12 | uint32_t(buffer[1]) << 4 | uint32_t(buffer[2] >>4);
}