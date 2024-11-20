cmake_minimum_required(VERSION 3.22)

# Pull in SDK (must be before project)
include(lib/pico-sdk/pico_sdk_init.cmake)

project(template C CXX ASM)
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

# Add all executables
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Pull in common dependencies
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
)

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create uf2 file
pico_add_uf2_output(${PROJECT_NAME})