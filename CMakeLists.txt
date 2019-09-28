cmake_minimum_required(VERSION 3.13)
project(mega-duck C ASM)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

add_subdirectory(firmware)
