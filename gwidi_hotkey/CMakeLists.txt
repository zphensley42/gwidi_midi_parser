cmake_minimum_required(VERSION 3.20)
project(gwidi_hotkey)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET gwidi_hotkey)
    set(gwidi_hotkey_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(gwidi_hotkey REQUIRED)
endif()

option(BUILD_TESTS "Build test exec" OFF)

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()
