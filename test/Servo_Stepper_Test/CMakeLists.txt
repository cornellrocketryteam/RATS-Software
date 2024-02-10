cmake_minimum_required(VERSION 3.22)

# Pull in SDK (must be before project)
include(../../lib/pico-sdk/pico_sdk_init.cmake)

project(servo_stepper C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add compiler flags and definitions
add_compile_options(
    -Wall
    -Wno-format          
    -Wno-unused-function
)

add_definitions(-DVERBOSE -DDEBUG)

# Add source files from src/
set(SOURCES
    servo_stepper.cpp
    ../../lib/pico-stepper/lib/stepper.c
    ../../lib/pico-servo/include/pico_servo.h
)

# Add all executables
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Add external libraries
add_subdirectory("../../lib/RadioLib" "${CMAKE_CURRENT_BINARY_DIR}/RadioLib")
add_subdirectory("../../lib/pico-stepper/lib" "${CMAKE_CURRENT_BINARY_DIR}/pico-stepper")
add_subdirectory("../../lib/pico-servo" "${CMAKE_CURRENT_BINARY_DIR}/pico-servo")

target_link_libraries(${PROJECT_NAME} 
    pico_stdlib 
    hardware_i2c 
    hardware_spi
    hardware_gpio
    hardware_pwm
    RadioLib
    pico-stepper
    fixmath
    pico-servo
)

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})