cmake_minimum_required(VERSION 3.20.2)
project(linux_sendinput)

set(CMAKE_CXX_STANDARD 17)

add_executable(linux_send_input_test main.cc)

add_library(linux_send_input)
target_sources(linux_send_input PUBLIC LinuxSendInput.cc)
target_include_directories(linux_send_input PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(linux_send_input_test linux_send_input)
