cmake_minimum_required(VERSION 3.22)

# Pull in SDK (must be before project)
include(../../lib/pico-sdk/pico_sdk_init.cmake)
include(GenerateExportHeader)

project(radio C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_definitions(-DRATS_VERBOSE)
# add_definitions(-DRATS_TIME)
add_definitions(-DSDIO)

# Add compiler flags and definitions
add_compile_options(
    -Wall
    -Wno-format          
    -Wno-unused-function
)

# Add all executables
add_executable(${PROJECT_NAME}
    # src_rfid/i2c_scanner.cpp
    # test/uart.cpp
    # radio.cpp
    main.cpp
    # ../../src/radio.cpp
    # ../../src/sd.cpp
    # hw_config.c
)

# Add external libraries
add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build_noos)
add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/influxdb/src build_influxdb)
add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/ublox-MX-Pico ${CMAKE_CURRENT_BINARY_DIR}/ublox-MX-Pico)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/../../src
    ${PROJECT_SOURCE_DIR}/../../lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src
    ${PROJECT_SOURCE_DIR}/../../lib/ublox-MX-Pico
    ${PROJECT_SOURCE_DIR}/../../lib/influxdb
)
# Pull in common dependencies
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_uart
    hardware_i2c
    hardware_gpio
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico 
    ublox-MX-Pico
    influxdb
)

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 
pico_add_extra_outputs(${PROJECT_NAME})

# Create uf2 file
pico_add_uf2_output(${PROJECT_NAME})



