cmake_minimum_required(VERSION 3.22)

# Pull in SDK (must be before project)
include(../../pico-sdk/pico_sdk_init.cmake)

project(simple C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(simple simple.c)

# Initialize the SDK
pico_sdk_init()

pico_enable_stdio_usb(simple 0)
pico_enable_stdio_uart(simple 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(simple)

target_include_directories(simple PRIVATE ../include)
target_link_directories(simple PRIVATE ../build)
target_link_libraries(simple PRIVATE pico_stdlib hardware_pwm pico-servo fixmath)