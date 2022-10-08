if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options/)
    find_package(gwidi_options REQUIRED)
endif()

if(NOT TARGET gwidi_data)
    set(gwidi_data_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_data/)
    find_package(gwidi_data REQUIRED)
endif()

if(NOT TARGET gwidi_midi)
    set(gwidi_midi_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_midi)
    find_package(gwidi_midi REQUIRED)
endif()

# Define targets
add_library(gwidi_tick)
target_sources(gwidi_tick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_tick_parser.cc
)

add_dependencies(gwidi_tick ${gwidi_options_LIBRARIES} ${gwidi_data_LIBRARIES} ${gwidi_midi_LIBRARIES})
target_include_directories(gwidi_tick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${gwidi_data_INCLUDE_DIRS}
        ${gwidi_options_INCLUDE_DIRS}
        ${gwidi_midi_INCLUDE_DIRS}
)
target_link_libraries(gwidi_tick PRIVATE
        spdlog::spdlog
        ${gwidi_data_LIBRARIES}
        ${gwidi_options_LIBRARIES}
        ${gwidi_midi_LIBRARIES}
)

set(gwidi_tick_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(gwidi_tick_LIBRARIES gwidi_tick)
