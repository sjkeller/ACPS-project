# Lab Notes

## BMP280 (Pressure & Temperature Sensor)

- **Pinout:**

  - The **SDA** and **SCL** lines are connected to the Lora Board on pins **PB_9** and **PB_8**, respectively.
    ~~~c
    #define I2C_SDA PB_9
    #define I2C_SCL PB_8
    ~~~
  - Pin **SDO** is connected to **GND** (determines the I2C address)
  
- To configure the I2C communication with the sensor refer to the **[BMP280 datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp280-ds001.pdf)**

- Make sure  to wake up the sensor (change operation mode from sleep mode to normal mode) before reading out the data.

- Use the functions provided in the [**temp_press_calculation.cpp**](temp_press_calculation.cpp) file to calculate the temperature and pressure values based on raw sensor data (ADC values received via I2C).

**Further Resources:**

- [BMP280 product page](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/)

- [Sensor Breakout Board](https://www.reichelt.de/entwicklerboards-temperatur-und-drucksensor-bmp280-debo-bmp280-p266034.html?&nbc=1&trstct=lsbght_sldr::266078)

  

## SD Card Data Logging

- The provided **`main.cpp`** contains an example of how to setup and write files on the SD card.

- Before running the program, make sure that you copy the **`mbed_app.json`** file to your project directory, which contains the configuration of the SPI connection.

- Also, you need to enable the **storage** feature for Mbed OS by commenting the following lines in **`.mbedignore`**:

  ~~~
  /* Storage */
  //mbed-os/storage/filesystem/*
  //mbed-os/storage/kvstore/*
  //mbed-os/storage/platform/*
  ~~~

  (or just copy the new provided .mbedignore file).

- First, run the program as is to make sure that the SD card will be formatted once. Afterwards you can disable the formatting by setting

  ~~~c++
  #define FORMAT_SD_CARD false
  ~~~

**Further Resources:**

- [SDBlockDevice](https://os.mbed.com/docs/mbed-os/v6.15/apis/sdblockdevice.html)

- [FATFileSystem](https://os.mbed.com/docs/mbed-os/v6.15/apis/fatfilesystem.html)

## Misc

For your interest, you can find a connection diagram here: [lora-board_connection-diagram.pdf](lora-board_connection-diagram.pdf)

- Please **DO NOT change connections** on the provided hardware!
- Everything should be properly connected already. If you find there might be wrong connection, please report it to a tutor.
