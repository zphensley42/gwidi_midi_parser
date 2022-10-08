cmake_minimum_required(VERSION 3.20)
project(gwidi_options)
set(version 1.0)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR})
    find_package(gwidi_options REQUIRED)
endif()

option(BUILD_TESTS "Build test exec" OFF)
message("gwidi_options build tests: ${BUILD_TESTS}")

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()

# Define installation for outside projects
#install(
#        FILES
#        include/GwidiOptions.h
#        include/util.h
#        DESTINATION
#        include/gwidi_options-${version}
#)
#
#install(
#        TARGETS gwidi_options inipp
#        DESTINATION lib/gwidi_options-${version}
#        EXPORT gwidi_options-targets
#)
#
#install(
#        EXPORT
#        gwidi_options-targets
#        DESTINATION lib/gwidi_options-${version}
#)
