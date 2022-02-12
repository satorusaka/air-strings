# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/jimmylye/GitHub/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include("PicoLed/PicoLed.cmake")

project(air_strings C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(air_strings)

target_sources(air_strings PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(air_strings PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

pico_set_program_name(air_strings "Chuni Air Strings")
pico_set_program_version(air_strings "0.1")

# enable usb output, disable uart output
pico_enable_stdio_usb(air_strings 1)
pico_enable_stdio_uart(air_strings 0)

# Add the library to the build
# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(air_strings PUBLIC pico_stdlib tinyusb_device tinyusb_board)


pico_add_extra_outputs(air_strings)
