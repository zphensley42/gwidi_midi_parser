cmake_minimum_required(VERSION 3.20)
project(gwidi_playback)

set(CMAKE_CXX_STANDARD 17)

add_library(gwidi_tick_parser)
target_sources(gwidi_tick_parser PUBLIC gwidi_tick_parser.cc)
target_include_directories(gwidi_tick_parser PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_SOURCE_DIR}/gwidi_options/include
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/midifile/include
        ${CMAKE_SOURCE_DIR}/spdlog/include
)
target_link_libraries(gwidi_tick_parser PUBLIC
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/libgwidi_midi_parser.a
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/gwidi_options/libgwidi_options.a
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/midifile/libmidifile.a
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/spdlog/libspdlogd.a
)

add_executable(gwidi_tick gwidi_tick.cc)
target_include_directories(gwidi_tick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
if(WIN32)
    target_include_directories(gwidi_tick PUBLIC ${CMAKE_SOURCE_DIR}/windows_sendinput)
    target_link_libraries(gwidi_tick PUBLIC gwidi_tick_parser windows_send_input)
elseif(WIN64)
    target_include_directories(gwidi_tick PUBLIC ${CMAKE_SOURCE_DIR}/windows_sendinput)
    target_link_libraries(gwidi_tick PUBLIC gwidi_tick_parser windows_send_input)
elseif(UNIX)
    target_include_directories(gwidi_tick PUBLIC ${CMAKE_SOURCE_DIR}/linux_sendinput)
    target_link_libraries(gwidi_tick PUBLIC gwidi_tick_parser linux_send_input)
endif()
