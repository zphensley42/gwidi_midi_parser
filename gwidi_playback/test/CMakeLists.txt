if(NOT TARGET gwidi_tick)
    set(gwidi_tick_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_tick REQUIRED)
endif()

add_executable(gwidi_tick_exec gwidi_tick_exec.cc)
target_include_directories(gwidi_tick_exec PUBLIC
        ${gwidi_tick_INCLUDE_DIRS}
)
target_link_libraries(gwidi_tick_exec PUBLIC ${gwidi_tick_LIBRARIES})
