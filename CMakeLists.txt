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

if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_options_2)
    find_package(gwidi_options_2 REQUIRED)
endif()

if(NOT TARGET gwidi_hotkey)
    set(gwidi_hotkey_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_hotkey)
    find_package(gwidi_hotkey REQUIRED)
endif()

if(NOT TARGET gwidi_server_client)
    set(gwidi_server_client_DIR ${CMAKE_CURRENT_LIST_DIR}/gwidi_server_client)
    find_package(gwidi_server_client REQUIRED)
endif()

if(BUILD_TESTS)
    message("BUILDING TESTS")
    add_subdirectory(gwidi_data/test)
    add_subdirectory(gwidi_options_2/test)
    add_subdirectory(gwidi_midi/test)
    add_subdirectory(gwidi_playback/test)
    add_subdirectory(gwidi_playback/udp_sendinput/test)
    add_subdirectory(gwidi_hotkey/test)
    add_subdirectory(strutil/test)
    add_subdirectory(gwidi_server_client/test)
endif()


set(INSTALL_LIB_DEST "lib/gwidi")
set(INSTALL_HEADER_DEST "include/gwidi")
set(INSTALL_BIN_DEST "bin/gwidi")

install(
        TARGETS ${gwidi_options_LIBRARIES} ${gwidi_data_LIBRARIES} ${gwidi_midi_LIBRARIES} ${gwidi_tick_LIBRARIES} ${gwidi_options_2_LIBRARIES} ${gwidi_hotkey_LIBRARIES} ${gwidi_server_client_LIBRARIES}
        DESTINATION ${INSTALL_LIB_DEST}
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
        FILES ${udp_sendinput_INCLUDE_DIRS}/UdpSendInput.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_hotkey_INCLUDE_DIRS}/gwidi_hotkey.h
        DESTINATION ${INSTALL_HEADER_DEST}
)

install(
        FILES ${gwidi_server_client_INCLUDE_DIRS}/GwidiServerClient.h
        DESTINATION ${INSTALL_HEADER_DEST}
)
