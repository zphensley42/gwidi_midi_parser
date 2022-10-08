cmake_minimum_required(VERSION 3.20.2)
project(windows_sendinput)

set(CMAKE_CXX_STANDARD 17)

add_executable(windows_send_input_test main.cc)

add_library(windows_send_input)
target_sources(windows_send_input PUBLIC WindowsSendInput.cc)
target_include_directories(windows_send_input PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(windows_send_input_test windows_send_input)
