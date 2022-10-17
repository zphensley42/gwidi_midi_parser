if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options/)
    find_package(gwidi_options REQUIRED)
endif()

if(NOT TARGET gwidi_options_2)
    set(gwidi_options_2_DIR ${CMAKE_CURRENT_LIST_DIR}/../gwidi_options_2/)
    find_package(gwidi_options_2 REQUIRED)
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
        ${gwidi_options_2_INCLUDE_DIRS}
        ${gwidi_midi_INCLUDE_DIRS}
)
target_link_libraries(gwidi_tick PRIVATE
        spdlog::spdlog
        ${gwidi_data_LIBRARIES}
        ${gwidi_options_LIBRARIES}
        ${gwidi_options_2_LIBRARIES}
        ${gwidi_midi_LIBRARIES}
)

set(sendinput_libs "")
set(sendinput_include "")

if(WIN32)
    target_include_directories(gwidi_tick PUBLIC ${CMAKE_SOURCE_DIR}/windows_sendinput)
    target_link_libraries(gwidi_tick PUBLIC windows_send_input)
elseif(WIN64)
    target_include_directories(gwidi_tick PUBLIC ${CMAKE_SOURCE_DIR}/windows_sendinput)
    target_link_libraries(gwidi_tick PUBLIC windows_send_input)
elseif(UNIX)
    message("Including linux_sendinput")
    set(linux_send_input_DIR ${CMAKE_CURRENT_LIST_DIR}/linux_sendinput)
    find_package(linux_send_input REQUIRED)
    target_include_directories(gwidi_tick PUBLIC ${linux_send_input_INCLUDE_DIRS})
    target_link_libraries(gwidi_tick PUBLIC ${linux_send_input_LIBRARIES})
    set(sendinput_libs ${linux_send_input_LIBRARIES})
    set(sendinput_include ${linux_send_input_INCLUDE_DIRS})
endif()


add_library(gwidi_playback)
add_dependencies(gwidi_playback gwidi_tick)
target_sources(gwidi_playback PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_playback.cc
)
target_link_libraries(gwidi_playback PUBLIC gwidi_tick)


set(gwidi_tick_INSTALL_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(gwidi_tick_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include ${sendinput_include})
set(gwidi_tick_LIBRARIES gwidi_tick gwidi_playback ${sendinput_libs})
