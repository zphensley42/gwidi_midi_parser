cmake_minimum_required(VERSION 3.20)
project(gwidi_midi_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_TESTS "Build test exec" OFF)

if(NOT TARGET gwidi_tick)
    set(gwidi_tick_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_playback)
    find_package(gwidi_tick REQUIRED)
endif()

if(NOT TARGET gwidi_midi)
    set(gwidi_midi_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_midi)
    find_package(gwidi_midi REQUIRED)
endif()

if(NOT TARGET gwidi_data)
    set(gwidi_data_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_data)
    find_package(gwidi_data REQUIRED)
endif()

if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_options)
    find_package(gwidi_options REQUIRED)
endif()

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(gwidi_options/test)
    add_subdirectory(gwidi_midi/test)
    add_subdirectory(gwidi_playback/test)
endif()