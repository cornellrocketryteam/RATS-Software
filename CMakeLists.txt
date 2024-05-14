cmake_minimum_required(VERSION 3.22)

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

# Add source files from src/
set(SOURCES
    src/main.cpp
    src/formulas.cpp
    src/radio.cpp
    src/mover.cpp
    lib/pico-stepper/lib/stepper.c
    lib/pico-servo/include/pico_servo.h
)

# Add all executables
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

add_executable(${PROJECT_NAME}-debug
    ${SOURCES}
)
target_compile_definitions(${PROJECT_NAME}-debug PUBLIC DEBUG)

add_executable(${PROJECT_NAME}-move
    ${SOURCES}
)
target_compile_definitions(${PROJECT_NAME}-move PUBLIC MOVER)

add_executable(${PROJECT_NAME}-move-debug
    ${SOURCES}
)
target_compile_definitions(${PROJECT_NAME}-move-debug PUBLIC MOVER)
target_compile_definitions(${PROJECT_NAME}-move-debug PUBLIC DEBUG)

# Add external libraries
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/RadioLib)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pico-stepper/lib stepper)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pico-servo)

# Add libraries
set(LIBS 
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

target_link_libraries(${PROJECT_NAME} ${LIBS})
target_link_libraries(${PROJECT_NAME}-debug ${LIBS})
target_link_libraries(${PROJECT_NAME}-move ${LIBS})
target_link_libraries(${PROJECT_NAME}-move-debug ${LIBS})

# Enable serial over USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME}-debug 1)
pico_enable_stdio_uart(${PROJECT_NAME}-debug 0)
pico_enable_stdio_usb(${PROJECT_NAME}-move 1)
pico_enable_stdio_uart(${PROJECT_NAME}-move 0)
pico_enable_stdio_usb(${PROJECT_NAME}-move-debug 1)
pico_enable_stdio_uart(${PROJECT_NAME}-move-debug 0)

# Create map/bin/hex file etc.
pico_add_uf2_output(${PROJECT_NAME})
pico_add_uf2_output(${PROJECT_NAME}-debug)
pico_add_uf2_output(${PROJECT_NAME}-move)
pico_add_uf2_output(${PROJECT_NAME}-move-debug)