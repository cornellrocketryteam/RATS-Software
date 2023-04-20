# RATS-Software
This is the code for receiving rocket telemetry, controlling the motor and servo that aim the antenna, and forwarding data along to the GUI laptop via serial.

# Instructions to run:

## Board Manager:
Earle F. Philhower, III (https://github.com/earlephilhower/arduino-pico)

## Libraries:
The only library needed at the moment is RadioLib which can be installed from Arduino IDE. The FEC Varients (WIP) require RS-FEC as well (https://github.com/simonyipeter/Arduino-FEC)

## Pins:

NSS (CS) pin:   1

DIO0 (Interrupt) pin:  5

RESET pin: 4 

DIO1 (Interrupt) pin:  6

RX (MISO) pin: 0

TX (MOSI) pin: 3

SCK pin: 2

### Code:

SX1276 radio = new Module(1, 5, 4, 6);

SPI.setRX(0);

SPI.setTX(3);

SPI.setCS(1);

SPI.setSCK(2);



