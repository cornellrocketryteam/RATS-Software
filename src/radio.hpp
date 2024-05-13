#include "constants.hpp"
#include "rfm/pico_hal.h"
#include <RadioLib.h>

class Radio {
public:
    int init();
    int receive(char *received, char *metadata);

private:
    PicoHal *hal = new PicoHal(SPI_PORT, constants::SPI_MISO, constants::SPI_MOSI, constants::SPI_SCK, 8000000);
    SX1276 radio = new Module(hal, constants::RX_CS, constants::RX_DIO0, RADIOLIB_NC, constants::RX_DIO1);
    uint8_t received[constants::MSG_LEN];
};