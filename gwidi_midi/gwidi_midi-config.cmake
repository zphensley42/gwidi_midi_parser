if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options/)
    find_package(gwidi_options REQUIRED)
endif()

if(NOT TARGET gwidi_data)
    set(gwidi_data_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_data/)
    find_package(gwidi_data REQUIRED)
endif()

if(NOT TARGET midifile)
    set(midifile_DIR ${CMAKE_CURRENT_LIST_DIR}/midifile)
    find_package(midifile REQUIRED)
    message("midifile_INCLUDE_DIRS: ${midifile_INCLUDE_DIRS}")
endif()

# Define targets
add_library(gwidi_midi)
target_sources(gwidi_midi PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_midi_parser.cc
)

add_dependencies(gwidi_midi ${midifile_LIBRARIES})
target_include_directories(gwidi_midi PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${gwidi_data_INCLUDE_DIRS}
        ${gwidi_options_INCLUDE_DIRS}
        ${midifile_INCLUDE_DIRS}
)
target_link_libraries(gwidi_midi PRIVATE
        spdlog::spdlog
        ${gwidi_data_LIBRARIES}
        ${gwidi_options_LIBRARIES}
        ${midifile_LIBRARIES}
)

set(gwidi_midi_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(gwidi_midi_LIBRARIES gwidi_midi ${midifile_LIBRARIES})
