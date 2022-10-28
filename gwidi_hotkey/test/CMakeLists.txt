if(NOT TARGET gwidi_hotkey)
    set(gwidi_hotkey_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_hotkey REQUIRED)
endif()

add_executable(gwidi_hotkey_exec gwidi_hotkey_exec.cc)
target_include_directories(gwidi_hotkey_exec PUBLIC
        ${gwidi_hotkey_INCLUDE_DIRS}
)
target_link_libraries(gwidi_hotkey_exec PUBLIC ${gwidi_hotkey_LIBRARIES})
