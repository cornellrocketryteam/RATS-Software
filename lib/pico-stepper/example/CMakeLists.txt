cmake_minimum_required(VERSION 3.22)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(../../pico-sdk/pico_sdk_init.cmake)

project(stepper-basic-example C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise the Raspberry Pi Pico SDK
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
	stepper-basic-example.c
)

# Add all executables
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Add pico-stepper directory
add_subdirectory(../lib stepper)

# Standard and additional libraries
target_link_libraries(${PROJECT_NAME}
	pico_stdlib
	pico-stepper)

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})