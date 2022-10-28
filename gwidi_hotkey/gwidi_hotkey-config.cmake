# Define targets
add_library(gwidi_hotkey)
target_sources(gwidi_hotkey PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/gwidi_hotkey.cc
)

target_include_directories(gwidi_hotkey PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(gwidi_hotkey PRIVATE
        spdlog::spdlog
)

# TODO: separate links to linux / windows versions that allow for this to work

set(gwidi_hotkey_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
set(gwidi_hotkey_LIBRARIES gwidi_hotkey)
