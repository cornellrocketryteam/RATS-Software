/**
 * @file sd_write_sdio.cpp
 * @author csg83
 *
 * @brief Unit test of SD writes over SDIO or SPI
 */

#include "pico/stdlib.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include "tusb.h"
#include "hardware/gpio.h"

int main()
{
    stdio_init_all();

    // Wait for serial monitor to open (not necessary for production code)
    printf("Starting\n");
    while (!tud_cdc_connected())
    {
        sleep_ms(500);
    }
    printf("Connected\n");

    // Mount file system
    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr)
    {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("Card mounted\n");

    // Open file
    FIL fil;
    const char *const filename = "test.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }
    printf("File opened\n");

    // Write to file
    if (f_printf(&fil, "Hello, world!\n") < 0)
    {
        printf("f_printf failed\n");
    }
    printf("Data written\n");

    // Close file
    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("File closed\n");

    // Unmount file system
    f_unmount("");
    printf("Card unmounted\n");

    return 0;
}