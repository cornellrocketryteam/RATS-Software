# cmake_minimum_required(VERSION 3.22)

# # Pull in SDK (must be before project)
# include(GenerateExportHeader)

# project(radio C CXX ASM)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)


# add_definitions(-DRATS_VERBOSE)
# # add_definitions(-DRATS_TIME)
# add_definitions(-DSDIO)

# # Add compiler flags and definitions
# add_compile_options(
#     -Wall
#     -Wno-format          
#     -Wno-unused-function
# )



# find_package(InfluxDB REQUIRED)

# # Use pkg-config to find libusb-1.0
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# # Add all executables
# add_executable(${PROJECT_NAME}
#     main.cpp
# )

# # Add external libraries
# # add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build_noos)
# # add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/influxdb/src build_influxdb)
# # add_subdirectory(${PROJECT_SOURCE_DIR}/../../lib/ublox-MX-Pico ${CMAKE_CURRENT_BINARY_DIR}/ublox-MX-Pico)


# target_include_directories(${PROJECT_NAME} PRIVATE
#     # ${PROJECT_SOURCE_DIR}/../../src
#     # ${PROJECT_SOURCE_DIR}/../../lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src
#     # ${PROJECT_SOURCE_DIR}/../../lib/ublox-MX-Pico
#     # ${PROJECT_SOURCE_DIR}/../../lib/influxdb
#     ${LIBUSB_1.0_INCLUDE_DIRS}
# )
# # Pull in common dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     # pico_stdlib
#     # hardware_uart
#     # hardware_i2c
#     # hardware_gpio
#     # no-OS-FatFS-SD-SDIO-SPI-RPi-Pico 
#     # ublox-MX-Pico
#     # influxdb
#     InfluxData::InfluxDB
#     ${LIBUSB_1.0_LIBRARIES}
# )


cmake_minimum_required(VERSION 3.22)
project(proxy C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(GenerateExportHeader)

add_definitions(-DRATS_VERBOSE)
add_definitions(-DSDIO)

add_compile_options(
    -Wall
    -Wno-format          
    -Wno-unused-function
)

find_package(InfluxDB REQUIRED)

# Use pkg-config to find libusb-1.0
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_executable(${PROJECT_NAME} main.cpp
usb.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBUSB_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/../../src
    .
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    InfluxData::InfluxDB
    ${LIBUSB_LIBRARIES}
)

# Optionally, if you need to propagate pkg-config flags:
target_compile_options(${PROJECT_NAME} PRIVATE ${LIBUSB_CFLAGS_OTHER})


