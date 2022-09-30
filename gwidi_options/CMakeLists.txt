cmake_minimum_required(VERSION 3.20)
project(gwidi_midi_parser)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

add_library(gwidi_options)
target_sources(gwidi_options PUBLIC gwidi_options_parser.cc)
target_include_directories(gwidi_options PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_SOURCE_DIR}/spdlog/include
        ${CMAKE_SOURCE_DIR}
)
target_link_libraries(gwidi_options PUBLIC
        spdlog::spdlog
)


add_executable(gwidi_options_exec gwidi_options.cc)
target_include_directories(gwidi_options_exec PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(gwidi_options_exec PUBLIC gwidi_options)

FetchContent_Declare(inipp
        GIT_REPOSITORY https://github.com/mcmtroffaes/inipp.git
        GIT_TAG        develop
)
FetchContent_MakeAvailable(inipp)

target_link_libraries(gwidi_options PUBLIC inipp::inipp)
