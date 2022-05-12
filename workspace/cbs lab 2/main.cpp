/*!
 *  Exercise 3
 */
#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include "BufferedSerial.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#define SDA_PIN PB_9
#define SCL_PIN PB_8
#define BME280 0x76

#define DEL_LOGS PB_2

#define FORMAT_SD_CARD 0


I2C bus(SDA_PIN, SCL_PIN);
SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI,
                 MBED_CONF_SD_SPI_MISO,
                 MBED_CONF_SD_SPI_CLK,
                 MBED_CONF_SD_SPI_CS);

FATFileSystem fs("sd", &sd);
static BufferedSerial monitor(USBTX, USBRX);

auto data_buffer = new char[8];
auto calib_data = new char[26];

int32_t temperature;
int32_t pressure;

int real_temp;
int real_pres;

int year, month, day, hour, minute, second;

float temp_fine;
short dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
unsigned short dig_T1, dig_P1;

static volatile bool del_state = false;

/*!
 *  Interrupt routine for deleting the log files
 */
void deleteLogs(void) {
    del_state = true;
}

/*!
 *  Calculates the temperature in °C from the given raw temperature
 *  \param raw_temp The temperature measurement from the raw data
 */
int32_t calc_temp(int32_t raw_temp){
    int32_t temp;
    int32_t temp_fine;
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
uint32_t calc_press(int32_t raw_press)
{
    uint32_t p;
    // 32-bit only calculations
    int32_t var1, var2;
    int32_t temp_fine;


    var1 = (((int32_t)temp_fine) >> 1) - (int32_t)64000;
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
}

/*!
 *  Writes to the BMP280 via I2C
 *  \param address  The address to write to
 *  \param value    The value to be written
 */
int writeBMP(char address, char value) {
    auto write_array = new char[2];
    write_array[0] = address;
    write_array[1] = value;
    return bus.write(BME280 << 1, write_array, 2);
}

/*!
 *  Reads from the BMP280 via I2C
 *  \param address  The address to read from
 *  \param data     The buffer to which the values will be read
 *  \param n        Number of bytes to read
 *  \return         Returns 0 on success
 */
int readBMP(char address, char* data, int n = 1) {
    int succ_write, succ_read;
    succ_write = bus.write(BME280 << 1, &address, 1);
    succ_read = bus.read((BME280 << 1) + 1, data, n);
    return succ_write + succ_read;
}

/*!
 *  Gets BMP280 calibration data from the given data
 *  \param data  The data, which contains the calibration data
 *  \param data  The index of the wanted calibration data
 *  \param n     If the calibration data is in unsigned format
 *  \return      The wanted calibration data
 */
int getCalib(char* data, int index, bool u = false){
    if (u)
        return (unsigned short) ((short) data[index] + ((short) data[index + 1] << 8));
    return ((short) data[index] + ((short) data[index + 1] << 8));
}

/*!
 *  Reads the data from the serial interface. String is ended via the "x" symbol
 *  \param year  The variable in which the year data should be stored
 *  \param month  The variable in which the month data should be stored
 *  \param day  The variable in which the day data should be stored
 *  \param hour  The variable in which the hour data should be stored
 *  \param minute  The variable in which the minute data should be stored
 *  \param second  The variable in which the second data should be stored
 */
int readDate(int *year, int *month, int *day, int *hour, int *minute, int *second) {
    auto cmd_buffer_c = new char[1];
    auto cmd_buffer = new char[50];
    int length;
    for (int j = 0; j < 50; j ++) {
        cmd_buffer[j] = 0;
    }
    int i = 0;
    while(i < 50) {
        length = monitor.read(cmd_buffer_c, sizeof(cmd_buffer_c));
        printf("%d", length);
        cmd_buffer[i] = *cmd_buffer_c;
        if (cmd_buffer[i] == 'x') {
            cmd_buffer[i] = '\0';
            break;
        }
        i ++;
    }

    sscanf(cmd_buffer, "%d-%d-%d_%d-%d-%d", year, month, day, hour, minute, second);
    return length;
}


int main()
{
    // wake up from sleep mode
    writeBMP(0xF4, (7 << 5) + (7 << 2) + 3);

    //Wait for wake up
    ThisThread::sleep_for(200ms);

    // get calibration data
    readBMP(0x88, calib_data, 24);

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

    //print calibration data
    printf("T1: %hu\n", dig_T1);
    printf("T2: %hi\n", dig_T2);
    printf("T3: %hi\n", dig_T3);
    printf("P1: %hu\n", dig_P1);
    printf("P2: %hi\n", dig_P2);
    printf("P3: %hi\n", dig_P3);
    printf("P4: %hi\n", dig_P4);
    printf("P5: %hi\n", dig_P5);
    printf("P6: %hi\n", dig_P6);
    printf("P7: %hi\n", dig_P7);
    printf("P8: %hi\n", dig_P8);
    printf("P9: %hi\n", dig_P9);

    //Set interrupt for button
    InterruptIn button(DEL_LOGS);
    button.fall(&deleteLogs);

    //mount SD-Card
    fs.mount(&sd);
    monitor.set_baud(9600);
    fflush(stdout);

    //Set current time to received UART time
    set_time(0);
    readDate(&year, &month, &day, &hour, &minute, &second);
    printf("%d-%d-%d_%d-%d-%d", year, month, day, hour, minute, second);
    struct tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    time_t current_time = mktime(&t);
    set_time(current_time);

    monitor.set_blocking(0);

    printf("\n begin");
    ThisThread::sleep_for(1s);

    //Format SD-Card if flag was set
    if (FORMAT_SD_CARD) {
        printf("Formatting the SD Card... ");
        fflush(stdout);
        fs.format(&sd);
    }

    //Generate a new log file with the correct name and header
    time_t live_time = time(NULL);
    printf("\n live time: %u", live_time);
    char path_buffer[50] = {0};
    strftime(path_buffer, 50, "/sd/%F_%H-%M-%S.log", localtime(&live_time));
    printf("\n%s", path_buffer);

    FILE *fp = fopen(path_buffer, "w+");
    printf("starting log write\n");
    fprintf(fp, "Time (s), Pressure (hPa), Temperature (degC)\n");
    fclose(fp);

    bool new_file_state = false;

    while(true) {
        //Open the last file, if no new file was requested
        if(!new_file_state) {
            fp = fopen(path_buffer, "a");
        }

        //Generate a new log file with the correct name and header, if a new file was requested
        if(new_file_state) {
            live_time = time(NULL);
            strftime(path_buffer, 50, "/sd/%F_%H-%M-%S.log", localtime(&live_time));
            FILE *fp = fopen(path_buffer, "w+");
            fprintf(fp, "Time (s), Pressure (hPa), Temperature (degC)\n");
            new_file_state = false;
            printf("\n new Logfile created");
        }

        // get raw register data
        readBMP(0xF7, data_buffer, 6);

        // construct raw data from registers
        pressure = ((int32_t) data_buffer[0] << 12) + ((int32_t) data_buffer[1] << 4) + ((int32_t) data_buffer[2] >> 4);
        temperature = ((int32_t) data_buffer[3] << 12) + ((int32_t) data_buffer[4] << 4) + ((int32_t) data_buffer[5] >> 4);

        // convert raw data to physical values
        real_pres = calc_press(pressure);
        real_temp = calc_temp(temperature);

        // get time
        time_t live_time = time(NULL);
        char logtime_buffer[50] = {0};
        strftime(logtime_buffer, 50, "%H:%M:%S", localtime(&live_time));

        // print data to file
        fprintf(fp, "%s, %d.%d, %d.%d\n", logtime_buffer, real_pres / 100, real_pres % 100, real_temp / 100, real_temp % 100);
        printf("\n%s, %d.%d, %d.%d\n", logtime_buffer, real_pres / 100, real_pres % 100, real_temp / 100, real_temp % 100);
        fclose(fp);

        // delete file if delete state entered via interrupt
        if(del_state) {
            printf("starting format ... ");
            fflush(stdout);
            fs.format(&sd);
            printf("formatting done\n");
            del_state = 0;
            break;
        }

        //Set buffers to read serial data to, user buffer gets refreshed each cycle, total_user_buffer stays
        auto user_buffer = new char[10];
        user_buffer[0] = '\0';
        auto total_user_buffer = new char[10];

        int total_length = 0;
        int length = monitor.read(user_buffer, sizeof(user_buffer));
        if(length > 0)
            total_length += length;
        strncat(total_user_buffer, user_buffer, length);

        printf("\n%d : %s - %s",total_length, total_user_buffer, user_buffer);

        //erase total_user_buffer if it is full
        if(total_length > 9) {
            total_length = 0;
            total_user_buffer[0] = '\0';
        }

        //erase all files, if "erase all" was entered via UART
        const char* erase_cmd = "erase all";
        if(strcmp(total_user_buffer, erase_cmd) == 0) {
            del_state = 1;
            total_length = 0;
            total_user_buffer[0] = '\0';
        }

        //Create a new log file, if "new log" was entered via UART
        const char* newfile_cmd = "new log";
        if(strcmp(total_user_buffer, newfile_cmd) == 0) {
            new_file_state = true;
            total_length = 0;
            total_user_buffer[0] = '\0';
        }

        ThisThread::sleep_for(1s);
    }

    fs.unmount();

}