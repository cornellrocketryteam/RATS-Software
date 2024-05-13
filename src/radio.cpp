#include "radio.hpp"
#include "rfm/pico_hal.h"
#include <RadioLib.h>

int Radio::init() {
    gpio_init(constants::RX_CS);
    gpio_set_dir(constants::RX_CS, GPIO_OUT);

    gpio_init(constants::RX_RST);
    gpio_set_dir(constants::RX_RST, GPIO_OUT);

    sleep_ms(10);
    gpio_put(constants::RX_RST, 0);
    sleep_ms(10);
    gpio_put(constants::RX_RST, 1);

#ifdef DEBUG
    printf("[SX1276] Initializing ... ");
#endif // DEBUG

    int state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
    if (state != RADIOLIB_ERR_NONE) {
        printf("failed, code %d\n", state);
        return -1;
    }

#ifdef DEBUG
    printf("success!\n");
#endif // DEBUG

    return 0;
}

int Radio::receive(char *received, char *metadata) {

#ifdef DEBUG
    printf("[SX1276] Waiting for incoming transmission ... ");
#endif // DEBUG

    int state = radio.receive((uint8_t *)received, constants::MSG_LEN);
    if (state == RADIOLIB_ERR_NONE) {

#ifdef DEBUG
        // packet was successfully received
        printf("success!");
        printf("\n");
#endif // DEBUG

        // Automatically Correct Frequency Error
        float rssi = radio.getRSSI();
        float snr = radio.getSNR();
        float freqErr = radio.getFrequencyError(true);

        // Package radio metadata
        memcpy(metadata, &rssi, sizeof(rssi));
        memcpy(metadata + sizeof(float), &snr, sizeof(snr));
        memcpy(metadata + (2 * sizeof(float)), &snr, sizeof(snr));

#ifdef DEBUG
        // print the RSSI (Received Signal Strength Indicator)
        // of the last received packet
        printf("[SX1278] RSSI:\t\t\t");
        printf("%f", rssi);
        printf(" dBm\n");

        // print the SNR (Signal-to-Noise Ratio)
        // of the last received packet
        printf("[SX1278] SNR:\t\t\t");
        printf("%f", snr);
        printf(" dB\n");

        // print frequency error
        // of the last received packet
        printf("[SX1278] Frequency error:\t");
        printf("%f", freqErr);
        printf(" Hz\n");
#endif // DEBUG
        return 0;
    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        // timeout occurred while waiting for a packet
#ifdef DEBUG
        printf("timeout!\n");
#endif // DEBUG
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
        // packet was received, but is malformed
#ifdef DEBUG
        printf("CRC error!\n");
#endif // DEBUG
    } else if (state == RADIOLIB_ERR_SPI_WRITE_FAILED) {
        // Likely something on board shorted. Restart radio module
        int restart_state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
        while (restart_state != RADIOLIB_ERR_NONE) {
            printf("failed, code %d\n", state);
            restart_state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
        }
    } else {
        // some other error occurred
#ifdef DEBUG
        printf("failed, code %d\n", state);
#endif // DEBUG
    }
    return -1;
}
