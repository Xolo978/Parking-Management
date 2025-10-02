cmake_minimum_required(VERSION 3.10)
project(ParkingLot C)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_STANDARD 11)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(parking_lot ${SOURCES})

find_package(raylib REQUIRED)

target_link_libraries(parking_lot
    raylib
    m
    dl
    pthread
    GL
    X11
    rt
)

