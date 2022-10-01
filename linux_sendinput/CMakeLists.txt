cmake_minimum_required(VERSION 3.23)
project(evdev_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(evdev_test main.cc)

add_library(linux_send_input)
target_sources(linux_send_input PUBLIC LinuxSendInput.cc)
target_include_directories(linux_send_input PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(evdev_test linux_send_input)
