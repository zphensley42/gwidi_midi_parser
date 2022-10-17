if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options/)
    find_package(gwidi_options REQUIRED)
endif()

if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options_2/)
    find_package(gwidi_options_2 REQUIRED)
endif()

set(DATA_HDRS ${CMAKE_CURRENT_LIST_DIR}/include)

# Define targets
add_library(gwidi_data)
target_sources(gwidi_data PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/GwidiMidiData.cc
        ${CMAKE_CURRENT_LIST_DIR}/GwidiGuiData.cc
        ${CMAKE_CURRENT_LIST_DIR}/GwidiDataConverter.cc
)
target_include_directories(gwidi_data PUBLIC
        ${DATA_HDRS}
        ${gwidi_options_INCLUDE_DIRS}
        ${gwidi_options_2_INCLUDE_DIRS}
)
target_link_libraries(gwidi_data PRIVATE
        spdlog::spdlog
        ${gwidi_options_LIBRARIES}
        ${gwidi_options_2_LIBRARIES}
)


set(gwidi_data_INCLUDE_DIRS ${DATA_HDRS})
set(gwidi_data_LIBRARIES gwidi_data)
