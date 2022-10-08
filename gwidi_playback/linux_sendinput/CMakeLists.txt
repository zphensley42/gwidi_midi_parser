cmake_minimum_required(VERSION 3.20.2)
project(linux_sendinput)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET linux_send_input)
    set(linux_send_input_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(linux_send_input REQUIRED)
endif()

add_executable(linux_send_input_test main.cc)
target_include_directories(linux_send_input_test PUBLIC ${linux_send_input_INCLUDE_DIRS})
target_link_libraries(linux_send_input_test ${linux_send_input_LIBRARIES})
