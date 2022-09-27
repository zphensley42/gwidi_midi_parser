cmake_minimum_required(VERSION 3.20)
project(gwidi_midi_parser)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(midifile)
add_subdirectory(spdlog)
add_subdirectory(gwidi_playback)

add_library(gwidi_midi_parser)
target_sources(gwidi_midi_parser PUBLIC gwidi_midi_parser.cc)
target_include_directories(gwidi_midi_parser PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        midifile/include
        spdlog/include
)
target_link_libraries(gwidi_midi_parser PUBLIC
        ${CMAKE_SOURCE_DIR}/libs/libmidifile.a
        ${CMAKE_SOURCE_DIR}/libs/libspdlogd.a
)

add_executable(gwidi_midi gwidi_midi.cc)
target_include_directories(gwidi_midi PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(gwidi_midi PUBLIC gwidi_midi_parser)
