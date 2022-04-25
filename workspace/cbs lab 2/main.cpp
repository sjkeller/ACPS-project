/*
 * An example of how to setup a filesystem and write a file on 
 * an SD card that is connected via SPI to the microcontroller.
 */
#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"


#define FORMAT_SD_CARD true

SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI,
                 MBED_CONF_SD_SPI_MISO,
                 MBED_CONF_SD_SPI_CLK,
                 MBED_CONF_SD_SPI_CS);

FATFileSystem fs("sd", &sd);


void check_error(int err){
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(err), -err);
    }
}


int main()
{
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
    
    printf("Mbed OS filesystem example done!\n");

}

