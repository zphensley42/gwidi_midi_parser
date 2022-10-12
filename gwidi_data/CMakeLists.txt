cmake_minimum_required(VERSION 3.20)
project(gwidi_data)
set(version 1.0)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options/)
find_package(gwidi_options REQUIRED)

set(gwidi_data_DIR ${CMAKE_CURRENT_LIST_DIR})
find_package(gwidi_data REQUIRED)

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(test)
endif()

# Define installation for outside projects
#install(
#        FILES
#        include/GwidiData.h
#        DESTINATION
#        include/gwidi_data-${version}
#)
#
#install(
#        TARGETS gwidi_data
#        DESTINATION lib/gwidi_data-${version}
#        EXPORT gwidi_data-targets
#)
#
#install(
#        EXPORT
#        gwidi_data-targets
#        DESTINATION lib/gwidi_data-${version}
#)
