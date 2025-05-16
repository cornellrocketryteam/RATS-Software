cmake_minimum_required(VERSION 3.22)
project(proxy C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

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

add_executable(${PROJECT_NAME} 
    main.cpp
    cat.cpp
    util.cpp
    database.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBUSB_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/../../src
    .
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    InfluxData::InfluxDB
    ${LIBUSB_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PRIVATE ${LIBUSB_CFLAGS_OTHER})


