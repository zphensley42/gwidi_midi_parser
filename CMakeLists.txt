cmake_minimum_required(VERSION 3.20)
project(gwidi_midi_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

add_subdirectory(linux_sendinput)
add_subdirectory(windows_sendinput)
add_subdirectory(midifile)
add_subdirectory(spdlog)
add_subdirectory(gwidi_playback)
add_subdirectory(gwidi_options)

add_library(gwidi_midi_parser)
target_sources(gwidi_midi_parser PUBLIC gwidi_midi_parser.cc GwidiData.cc)
target_include_directories(gwidi_midi_parser PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        midifile/include
        spdlog/include
)
target_link_libraries(gwidi_midi_parser PUBLIC
        spdlog::spdlog
        ${CMAKE_SOURCE_DIR}/cmake-build-debug/midifile/libmidifile.a
        gwidi_options
)


add_executable(gwidi_midi gwidi_midi.cc)
target_include_directories(gwidi_midi PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(gwidi_midi PUBLIC gwidi_midi_parser)

FetchContent_Declare(inipp
        GIT_REPOSITORY https://github.com/mcmtroffaes/inipp.git
        GIT_TAG        develop
)
FetchContent_MakeAvailable(inipp)

target_link_libraries(gwidi_midi_parser PUBLIC inipp::inipp)
