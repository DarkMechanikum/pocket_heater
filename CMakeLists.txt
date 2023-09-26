cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME pocket_heater)

set(${PROJECT_NAME}_BOARD nano)
set(ARDUINO_CPU atmega328)
project(${PROJECT_NAME})

# Define additional source and header files or default arduino sketch files
# These files are now added to the sketch file, if the sketch is defined.

### Additional static libraries to include in the target.

### Main sketch file
set(${PROJECT_NAME}_SKETCH pocket_heater.ino)

### Add project directories into the build

### Additional settings to add non-standard or your own Arduino libraries.
# For this example (libs will contain additional arduino libraries)
# An Arduino library my_lib will contain files in libs/my_lib/: my_lib.h, my_lib.cpp + any other cpp files

# This is no longer need but left in for those occasions where automatics do not work out. 
# Current Arduino.cmake will recurse into any libraries which contain source files in src/ subdirectory. 
# Files in subdirectories whose name starts with cmake-build or examples are excluded
# For nested library sources replace ${LIB_NAME} with library name for each library
# set(_RECURSE true)

#### Additional settings for programmer. From programmers.txt
set(${PROJECT_NAME}_PROGRAMMER avrisp)

## Verbose build process

generate_arduino_firmware(${PROJECT_NAME})


print_board_list()
print_programmer_list()
