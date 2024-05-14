# RATS-Software
This is the code for receiving rocket telemetry, controlling the motor and servo that aim the antenna, and forwarding data along to the GUI laptop via serial.

## Building:
Use CMake to build by running `cmake .. && make` from the `RATS-Software/build` directory. You can make this directory by running `mkdir build` if needed.

There are two flags to be aware of: DEBUG and MOVER. Both are currently disabled by default. Enabling DEBUG will print verbose information such as the radio transmission metadata and the calculated relative position of the rocket. Enabling MOVER will actuate the servo and stepper to aim the antenna at the calculated position. To toggle these flags, add the `-D<FLAG>=[ON|OFF]` to the cmake comment. For example:

`cmake -DDEBUG=ON -DMOVER=ON .. && make`

`cmake -DDEBUG=ON .. && make`

`cmake -DMOVER=ON .. && make`

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
