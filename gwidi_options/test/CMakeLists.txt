if(NOT TARGET gwidi_options)
    set(gwidi_options_DIR ${CMAKE_CURRENT_LIST_DIR}/../)
    find_package(gwidi_options REQUIRED)
endif()

add_executable(gwidi_options_exec gwidi_options_exec.cc)
target_include_directories(gwidi_options_exec PUBLIC
        ${gwidi_options_INCLUDE_DIRS}
)
target_link_libraries(gwidi_options_exec PRIVATE ${gwidi_options_LIBRARIES})
