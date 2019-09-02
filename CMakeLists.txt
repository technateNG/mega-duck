cmake_minimum_required(VERSION 3.14)
project(mega-duck C ASM)

set(MCU "atmega8")
set(F_CPU "16000000")

set(COMMON_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU}")
set(COMMON_COMPILER /usr/bin/avr-gcc)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

set(CMAKE_C_COMPILER ${COMMON_COMPILER})
set(CMAKE_ASM_COMPILER ${COMMON_COMPILER})
set(CMAKE_C_FLAGS ${COMMON_FLAGS})
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS} -x assembler-with-cpp")

add_executable(mega-duck)
target_sources(mega-duck
        PRIVATE
            main.c
            usbdrv/usbdrv.c
            usbdrv/usbdrvasm.S
        )
target_include_directories(mega-duck PRIVATE ${CMAKE_SOURCE_DIR}/usbdrv)
target_compile_features(mega-duck PRIVATE c_std_99)

if (EXISTS $<TARGET_FILE:mega-duck>.hex)
    file(REMOVE $<TARGET_FILE:mega-duck>.hex)
endif()

add_custom_command(TARGET mega-duck POST_BUILD
        COMMAND
        avr-objcopy -j .text -j .data -O ihex $<TARGET_FILE:mega-duck> $<TARGET_FILE:mega-duck>.hex)