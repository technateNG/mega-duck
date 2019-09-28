cmake_minimum_required(VERSION 3.15)
project(connector C)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(libusb-1.0 REQUIRED)

add_executable(duck-connector)
target_sources(duck-connector PRIVATE connector.c)
target_compile_features(duck-connector PRIVATE c_std_99)
target_include_directories(duck-connector PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
target_link_libraries(duck-connector PRIVATE ${LIBUSB_1_LIBRARIES})
