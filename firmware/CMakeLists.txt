set(MCU "atmega8")
set(F_CPU "16000000")

set(COMMON_FLAGS "-mmcu=${MCU};-DF_CPU=${F_CPU}")

add_executable(mega-duck)
target_sources(mega-duck
        PRIVATE
        main.c
        usbdrv.c
        usbdrvasm.S
        )

set_source_files_properties(usbdrv/usbdrvasm.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
target_include_directories(mega-duck PRIVATE ${CMAKE_SOURCE_DIR}/usbdrv)
target_compile_features(mega-duck PRIVATE c_std_99)
target_compile_options(mega-duck PRIVATE ${COMMON_FLAGS})
target_link_options(mega-duck PRIVATE ${COMMON_FLAGS})

if (EXISTS $<TARGET_FILE:mega-duck>.hex)
    file(REMOVE $<TARGET_FILE:mega-duck>.hex)
endif()

add_custom_command(TARGET mega-duck POST_BUILD
        COMMAND
        avr-objcopy -j .text -j .data -O ihex $<TARGET_FILE:mega-duck> $<TARGET_FILE:mega-duck>.hex)
