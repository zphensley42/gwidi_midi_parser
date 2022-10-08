cmake_minimum_required(VERSION 3.20)
project(gwidi_playback)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET gwidi_tick)
    set(gwidi_tick_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(gwidi_tick REQUIRED)
endif()

option(BUILD_TESTS "Build test exec" OFF)

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()


#add_custom_command(TARGET gwidi_tick POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/cmake-build-debug/midifile/libmidifile.a"
#        "${CMAKE_SOURCE_DIR}/../gwidi_native_module/external/bin")
#
#add_custom_command(TARGET gwidi_tick POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/cmake-build-debug/spdlog/libspdlogd.a"
#        "${CMAKE_SOURCE_DIR}/../gwidi_native_module/external/bin")
#
#add_custom_command(TARGET gwidi_tick POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/cmake-build-debug/gwidi_options/libgwidi_options.a"
#        "${CMAKE_SOURCE_DIR}/../gwidi_native_module/external/bin")
#
#add_custom_command(TARGET gwidi_tick POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/cmake-build-debug/libgwidi_midi_parser.a"
#        "${CMAKE_SOURCE_DIR}/../gwidi_native_module/external/bin")
#
#add_custom_command(TARGET gwidi_tick POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/cmake-build-debug/gwidi_playback/libgwidi_tick_parser.a"
#        "${CMAKE_SOURCE_DIR}/../gwidi_native_module/external/bin")

