cmake_minimum_required(VERSION 3.22)

OPTION(SERVO "Enables Actuating the Servo" OFF) # Enabled by default
OPTION(STEPPER "Enables Rotating the Stepper" OFF) # Enabled by default
OPTION(DEBUG "Prints debug information" OFF) # Disabled by default

IF(SERVO)
    ADD_DEFINITIONS(-DSERVO)
ENDIF(SERVO)

IF(STEPPER)
    ADD_DEFINITIONS(-DSTEPPER)
ENDIF(STEPPER)

IF(DEBUG)
    ADD_DEFINITIONS(-DDEBUG)
ENDIF(DEBUG)

# Pull in SDK (must be before project)
include(lib/pico-sdk/pico_sdk_init.cmake)

project(rats C CXX ASM)
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

# add_definitions(-DVERBOSE -DDEBUG)

# Add source files from src/
set(SOURCES
    src/main.cpp
    src/formulas.cpp
    src/radio.cpp
    lib/pico-stepper/lib/stepper.c
    lib/pico-servo/include/pico_servo.h
)

# Add all executables
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Add external libraries
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/RadioLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pico-stepper/lib stepper)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pico-servo)

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