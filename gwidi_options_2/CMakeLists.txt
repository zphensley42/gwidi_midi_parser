cmake_minimum_required(VERSION 3.20)
project(gwidi_options_2)
set(version 1.0)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(gwidi_options_2 REQUIRED)
endif()

option(BUILD_TESTS "Build test exec" OFF)
message("gwidi_options_2 build tests: ${BUILD_TESTS}")

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()