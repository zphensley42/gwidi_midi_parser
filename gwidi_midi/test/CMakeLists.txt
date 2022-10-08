if(NOT TARGET gwidi_midi)
    set(gwidi_midi_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_midi REQUIRED)
endif()

add_executable(gwidi_midi_exec gwidi_midi.cc)
target_link_libraries(gwidi_midi_exec PUBLIC gwidi_midi)
