cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PuzzleSolver_MCU)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/actuators/piston.c
    Core/Src/actuators/emergency_stop.c
    Core/Src/actuators/buttons.c
    Core/Src/actuators/magnet.c
    Core/Src/actuators/rotator.c
    Core/Src/actuators/step_generator.c
    Core/Src/actuators/stepper.c
    Core/Src/actuators/homer.c
    Core/Src/actuators/limit_switch.c
    Core/Src/communication/package.c
    Core/Src/communication/puzzle.pb.c
    Core/Src/communication/uart_receiver.c
    Core/Src/communication/command_dispatcher.c
    Core/third_party/nanopb/pb_common.c
    Core/third_party/nanopb/pb_decode.c
    Core/third_party/nanopb/pb_encode.c
    Core/Src/coordination/motion_planner.c
    Core/Src/coordination/state_machine.c
    Core/Src/system/app.c
    Core/Src/system/sys_init.c
    Core/Src/system/interrupt.c
    Core/Src/tests/test_cli.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Inc/actuators
    Core/Inc/communication
    Core/Inc/coordination
    Core/Inc/system
    Core/Inc/tests
    Core/third_party/nanopb
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
 
# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

