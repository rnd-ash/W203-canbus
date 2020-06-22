cmake_minimum_required(VERSION 3.17)
project(MEGA_CODE)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(MEGA_CODE
    Bluetooth.cpp
    Bluetooth.h
    Canbus.cpp
    Canbus.h
    helper.h
    ICDisplay.cpp
    ICDisplay.h
    Kombi.cpp
    Kombi.h
    Lights.cpp
    Lights.h
    MEGA_CODE.ino
    Music.cpp
    Music.h)
