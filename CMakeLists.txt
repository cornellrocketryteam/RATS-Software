cmake_minimum_required(VERSION 3.22)

# Pull in SDK (must be before project)
include(lib/pico-sdk/pico_sdk_init.cmake)

project(rats C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# Antenna configuration checks use C++ exceptions.
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialize the SDK
pico_sdk_init()

# USB carries packed binary telemetry. Text diagnostics would corrupt that stream.

# Add compiler flags and definitions
add_compile_options(
    -Wall
    -Wno-format
    -Wno-unused-function
)

add_definitions(-DUSBD_PRODUCT="RATS Pico")

# Add all executables
add_executable(${PROJECT_NAME}
    src/radio.cpp
    src/main.cpp
    src/motor.cpp
    src/station.cpp
    src/usb.cpp
    src/sd.cpp
    src/hw_config.c
)

# Add external libraries
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/ublox-MX-Pico)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src
    ${PROJECT_SOURCE_DIR}/lib/ublox-MX-Pico
)
# Pull in common dependencies
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_uart
    hardware_i2c
    hardware_gpio
    hardware_dma
    pico_multicore
    pico_sync
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
    ublox-MX-Pico
)

# Core 0 uses SCRATCH_Y (4 KB). Core 1 receives its own static stack in motor.cpp.
target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_STACK_SIZE=0x1000)

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

# Create uf2 file
pico_add_uf2_output(${PROJECT_NAME})
