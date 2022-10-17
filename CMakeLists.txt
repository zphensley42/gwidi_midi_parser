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

if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_options_2)
    find_package(gwidi_options_2 REQUIRED)
endif()

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(gwidi_data/test)
    add_subdirectory(gwidi_options/test)
    add_subdirectory(gwidi_options_2/test)
    add_subdirectory(gwidi_midi/test)
    add_subdirectory(gwidi_playback/test)
endif()


set(INSTALL_LIB_DEST "lib/gwidi")
set(INSTALL_HEADER_DEST "include/gwidi")
set(INSTALL_BIN_DEST "bin/gwidi")

install(
        TARGETS ${gwidi_options_LIBRARIES} ${gwidi_data_LIBRARIES} ${gwidi_midi_LIBRARIES} ${gwidi_tick_LIBRARIES} ${gwidi_options_2_LIBRARIES}
        DESTINATION ${INSTALL_LIB_DEST}
)

install(
        FILES ${gwidi_options_INCLUDE_DIRS}/GwidiOptions.h ${gwidi_options_INCLUDE_DIRS}/GwidiOptions_Util.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_options_2_INCLUDE_DIRS}/GwidiOptions2.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_data_INCLUDE_DIRS}/GwidiMidiData.h ${gwidi_data_INCLUDE_DIRS}/GwidiGuiData.h ${gwidi_data_INCLUDE_DIRS}/GwidiDataConverter.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_midi_INCLUDE_DIRS}/gwidi_midi_parser.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_tick_INSTALL_INCLUDE_DIRS}/GwidiTickHandler.h ${gwidi_tick_INSTALL_INCLUDE_DIRS}/GwidiPlayback.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${linux_send_input_INCLUDE_DIRS}/LinuxSendInput.h
        DESTINATION ${INSTALL_HEADER_DEST}
)
