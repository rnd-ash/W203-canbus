cmake_minimum_required(VERSION 3.15)
project(w203_canbus)

set(CMAKE_CXX_STANDARD 98)

add_library(w203_canbus ic.cpp ic.h can.h icPacketBatch.cpp icPacketBatch.h)