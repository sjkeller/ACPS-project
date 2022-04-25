#include "mbed.h"

  /**
   * @brief Calculate temperature from raw value
   *
   * This function is based on the compensation formula given in the BMP280
   * datasheet.
   *
   * @param raw_temp Raw temperature value
   * @return Compensated temperature in (centidegrees Celsius = degC * 100)
   */
  int32_t calc_temp(int32_t raw_temp)
  {
    int32_t temp;
#if (BMP280_CALC_TYPE_INTEGER)
    // Integer calculations
    int temp_fine;

    temp_fine = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    temp_fine += (((((raw_temp >> 4) - ((int32_t)dig_T1)) * ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    temp = ((temp_fine * 5) + 128) >> 8;
    return temp;
#else
    // Float calculations
    float temp_fine;
    float var1, var2;

    var1 = (((float)raw_temp) / 16384.0F - ((float)dig_T1) / 1024.0F) * ((float)dig_T2);
    var2 = ((float)raw_temp) / 131072.0F - ((float)dig_T1) / 8192.0F;
    var2 = (var2 * var2) * ((float)dig_T3);
    temp_fine = var1 + var2;
    temp = (int32_t)((temp_fine / 5120.0F) * 100.0F);
    return temp;
#endif
  }

  /**
   * @brief Calculate pressure from raw value
   *
   * Call the temperature compensation @see calc_temp() and parse the return
   * value as temp input to this function. This function is based on the
   * compensation formula given in the BMP280 datasheet.
   *
   * @param raw_press Raw pressure value
   * @param temp Compensated temperature value (centidegrees Celsius)
   * @return Compensated pressure in milli Pascal
   */
  uint32_t calc_press(int32_t raw_press, int32_t temp)
  {
    uint32_t p;
#if (BMP280_CALC_TYPE_INTEGER)
    // 32-bit only calculations
    int32_t var1, var2;

    var1 = (((int32_t)temp) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = (((32768 + var1)) * ((int32_t)dig_P1)) >> 15;
    if (var1 == 0)
    {
      // avoid exception caused by division by zero
      return 0;
    }
    p = (((uint32_t)(((int32_t)1048576) - raw_press) - (uint32_t)(var2 >> 12))) * 3125U;
    if (p < 0x80000000U)
    {
      p = (p << 1) / ((uint32_t)var1);
    }
    else
    {
      p = (p / (uint32_t)var1) << 1;
    }
    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
    //   p = p * 1000U; // convert to mPa
    return p;
#else
    // Float calculations
    float var1, var2, p_f;
    float temp_f = (float)temp;
    var1 = (temp_f / 2.0F) - 64000.0F;
    var2 = var1 * var1 * ((float)dig_P6) / 32768.0F;
    var2 = var2 + var1 * ((float)dig_P5) * 2.0F;
    var2 = (var2 / 4.0F) + (((float)dig_P4) * 65536.0F);
    var1 = (((float)dig_P3) * var1 * var1 / 524288.0F + ((float)dig_P2) * var1) / 524288.0F;
    var1 = (1.0F + var1 / 32768.0F) * ((float)dig_P1);

    p_f = 1048576.0F - (float)raw_press;

    if ((uint32_t)var1 == 0U)
    {
      // Avoid exception caused by division by zero
      return 0;
    }
    p_f = (p_f - (var2 / 4096.0F)) * 6250.0F / var1;
    var1 = ((float)dig_P9) * p_f * p_f / 2147483648.0F;
    var2 = p_f * ((float)dig_P8) / 32768.0F;
    p_f += (var1 + var2 + ((float)dig_P7)) / 16.0F;
    //   p = (uint32_t)(p_f * 1000.0F); //convert to mPa
    p = (uint32_t)(p_f);
    return p;
#endif
  }

