/*
 * An example of how to setup a filesystem and write a file on 
 * an SD card that is connected via SPI to the microcontroller.
 */
#include "mbed.h"
#include <cstdint>
#include <cstdio>
//#include "SDBlockDevice.h"
//#include "FATFileSystem.h"

#define EX_SELECT 1

#define SDA_PIN PB_9
#define SCL_PIN PB_8
#define BME280 0x76

#define DEL_LOGS PB_2

#define FORMAT_SD_CARD true

/*SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI,
                 MBED_CONF_SD_SPI_MISO,
                 MBED_CONF_SD_SPI_CLK,
                 MBED_CONF_SD_SPI_CS);
*/
//FATFileSystem fs("sd", &sd);


/*void check_error(int err){
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(err), -err);
    }
}
*/
I2C bus(SDA_PIN, SCL_PIN);

auto write_buffer = new char[2];

auto data_buffer = new char[8];
auto calib_data = new char[26];

int32_t temperature;
int32_t pressure;

int real_temp;
int real_pres;

float temp_fine;
short dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
unsigned short dig_T1, dig_P1;

char address;

static volatile bool del_state = false;

void deleteLogs(void) {
    del_state = true;
}

int deletePath(const char *fsrc)
{
    int res;
    DIR *d = opendir(fsrc);
    struct dirent *p;
    char path[30] = {0};
    while((p = readdir(d)) != NULL) {
        strcpy(path, fsrc);
        strcat(path, "/");
        strcat(path, p->d_name);
        res = remove(path);
    }
    closedir(d);
    res = res + remove(fsrc);
    return res;
}

int32_t calc_temp(int32_t raw_temp){
    int32_t temp;
    int32_t temp_fine;
    // Integer calculations
    temp_fine = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    temp_fine += (((((raw_temp >> 4) - ((int32_t)dig_T1)) * ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    temp = ((temp_fine * 5) + 128) >> 8;
    return temp;
}

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


int writeBMP(char address, char value) {
    auto write_array = new char[2];
    write_array[0] = address;
    write_array[1] = value;
    return bus.write(BME280 << 1, write_array, 2);
}

int readBMP(char address, char* data, int n = 1) {
    int succ_write, succ_read;
    succ_write = bus.write(BME280 << 1, &address, 1);
    succ_read = bus.read((BME280 << 1) + 1, data, n);
    return succ_write + succ_read;
}

int getCalib(char* data, int index, bool u = false){
    if (u)
        return (unsigned short) ((short) data[index] + ((short) data[index + 1] << 8));
    return ((short) data[index] + ((short) data[index + 1] << 8));
}


//dig_T1 = (unsigned short) ((short) calib_data[0] + ((short) calib_data[1] << 8));
//dig_T1 = (short) ((short) calib_data[0] + ((short) calib_data[1] << 8));

int main()
{

    #if EX_SELECT == 1

        // wake up from sleep mode
        writeBMP(0xF4, (7 << 5) + (7 << 2) + 3);

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
        

        while(true) {

            

            // get raw ensor data
            readBMP(0xF7, data_buffer, 6);

            pressure = ((int32_t) data_buffer[0] << 12) + ((int32_t) data_buffer[1] << 4) + ((int32_t) data_buffer[2] >> 4);
            temperature = ((int32_t) data_buffer[3] << 12) + ((int32_t) data_buffer[4] << 4) + ((int32_t) data_buffer[5] >> 4);


            

            for (int i = 0; i < 6; i ++) {
                real_pres = calc_press(pressure);
                printf("Luftdruck: %d,%d hPa\n", real_pres / 100, real_pres % 100);
                real_temp = calc_temp(temperature);
                printf("Temperatur: %d,%d °C\n", real_temp / 100, real_temp % 100);


            }

            // convert raw data to physical values

            //0x88 - 0xA1


            


        // print out converted data
            ThisThread::sleep_for(500ms);
        }


    #endif

    #if EX_SELECT == 2


        InterruptIn button(DEL_LOGS);

        int err = 0;
        printf("Welcome to the filesystem example.\n");

        // Try to mount the filesystem
        printf("Mounting the filesystem... \n");

        err = fs.mount(&sd);    
        /* For some reason an error will be returned at this point,
        * however, you can ignore it and leave the next line commented
        */
        // check_error(err); 

        if (FORMAT_SD_CARD) {
            printf("Formatting the SD Card... ");
            fflush(stdout);
            err = fs.format(&sd);
            check_error(err);
        }

        set_time(1256729737);

        FILE *fp = fopen("/sd/datalog.txt", "w+");

        while(true) {
            fprintf(fp, "Time (s), Pressure (hPa), Temperature (degC)\n");

            time_t logtime = time(NULL);

            fprintf(fp, "%d, %d.%d, %d.%d", logtime, real_pres / 100, real_pres % 100, real_temp / 100, real_temp % 100);

            if(del_state) {
                deletePath("/sd/")
            }

            
            ThisThread::sleep_for(1s);
        }

        fclose(fp);
        fflush(stdout);
        err = fs.unmount();
        check_error(err);

        /*
        printf("Opening a new file, numbers.txt... ");
        FILE *fd = fopen("/sd/numbers.txt", "w+");
        printf("%s\n", (!fd ? "Fail :(" : "OK")); 

        for (int i = 0; i < 20; i++) {
            printf("Writing decimal numbers to a file (%d/20)\r", i);
            fprintf(fd, "%d\n", i);
        }
        printf("Writing decimal numbers to a file (20/20) done.\n");

        printf("Closing file...");
        fclose(fd);
        printf(" done.\n");


        printf("Re-opening file read-only... ");
        fd = fopen("/sd/numbers.txt", "r");    
        printf("%s\n", (!fd ? "Fail :(" : "OK")); 

        printf("Dumping file to screen.\n");
        char buff[16] = {0};
        while (!feof(fd)) {
            int size = fread(&buff[0], 1, 15, fd);
            fwrite(&buff[0], 1, size, stdout);
        }
        printf("EOF.\n");

        printf("Closing file...");
        fclose(fd);
        printf(" done.\n");


        printf("Opening root directory... ");
        DIR *dir = opendir("/sd/");
        printf("%s\n", (!dir ? "Fail :(" : "OK"));

        struct dirent *de;
        printf("Printing all filenames:\n");
        while ((de = readdir(dir)) != NULL) {
            printf("  %s\n", &(de->d_name)[0]);
        }

        printf("Closing root directory... ");
        err = closedir(dir);
        check_error(err);


        // Tidy up
        printf("Unmounting... ");
        fflush(stdout);
        err = fs.unmount();
        check_error(err);
        
        printf("Mbed OS filesystem example done!\n");*/
    #endif

}

