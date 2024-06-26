# RATS-Software
This is the code for receiving rocket telemetry, controlling the motor and servo that aim the antenna, and forwarding data along to the GUI laptop via serial.

## Getting Started
### Required Dependencies
* ```cmake```
* ```gcc-arm-embedded``` (for macOS)
* ```gcc-arm-none-eabi``` (for Linux)

### Optional Dependencies
* ```doxygen```: For documentation generation
* ```clang-format```: For formatting code

_These dependencies can be install via [Homebrew](https://brew.sh/) on macOS or [apt](https://ubuntu.com/server/docs/package-management) on WSL._
  
### Setup
1. Install the required dependencies
2. Run ```git submodule update --init``` from within ```lib/```
3. Run ```git submodule update --init``` from within ```lib/pico-sdk/```
4. Run ```git submodule update --init``` from within ```lib/pico-servo/```

## Building:
Use CMake to build by running `cmake .. && make` from the `RATS-Software/build` directory. You can make this directory by running `mkdir build` if needed.

There are two flags to be aware of: DEBUG and MOVER. DEBUG prints verbose information such as the radio transmission metadata and the calculated relative position of the rocket. MOVER actuates the servo and stepper to aim the antenna at the calculated position. The normal cmake commands builds four targets, one for each permutation of these flags:
1. rats
2. rats-debug
3. rats-move
4. rats-move-debug

Where each target

## Board Manager:
[pico-sdk](https://github.com/raspberrypi/pico-sdk)

## Libraries:
1. [RadioLib](https://github.com/jgromes/RadioLib)
2. [Pico Stepper](https://github.com/antgon/pico-stepper)
3. [Pico Servo](https://github.com/irishpatrick/pico-servo)

## Pins:

| Name             | Number  |
| ---------------- | ------- |
| NSS (CS)         | 5       |
| DIO0 (Interrupt) | 13      |
| RESET            | 0       |
| DIO1 (Interrupt) | 14      |
| RX (MISO)        | 4       |
| TX (MOSI)        | 3       |
| SCK              | 2       |
| Servo            | 12      |
| Stepper 1A       | 7       |
| Stepper 1B       | 8       |
| Stepper 2A       | 9       |
| Stepper 2B       | 10      |
